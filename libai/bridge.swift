import Foundation
import FoundationModels

// MARK: - Error Codes

/// Status codes indicating the availability of Apple Intelligence models.
@available(macOS 26.0, *)
public enum AIAvailabilityStatus: Int32 {
    case available = 1
    case deviceNotEligible = -1
    case intelligenceNotEnabled = -2
    case modelNotReady = -3
    case unknownError = -99
}

/// Error codes returned by AI Bridge operations.
@available(macOS 26.0, *)
public enum AIBridgeErrorCode: Int32 {
    case success = 0
    case modelUnavailable = -1
    case invalidJSON = -2
    case invalidInput = -3
    case encodingError = -4
    case sessionNotFound = -5
    case streamNotFound = -6
    case guardrailViolation = -7
    case toolExecutionError = -8
    case toolNotFound = -9
    case unknownError = -99
}

// MARK: - Session Configuration

/// Configuration parameters for AI session creation.
@available(macOS 26.0, *)
public struct SessionConfig: Codable {

}

// MARK: - Session Management

@available(macOS 26.0, *)
private class SessionInfo {
    let bridgeSession: LanguageModelSession
    let config: SessionConfig
    var toolCallbacks: [String: ToolCallback]

    struct ToolCallback {
        let callback:
            @convention(c) (UnsafePointer<CChar>, UnsafeRawPointer?) -> UnsafeMutablePointer<CChar>?
        let userData: UnsafeRawPointer?
    }

    init(
        session: LanguageModelSession,
        config: SessionConfig,
        toolCallbacks: [String: ToolCallback] = [:]
    ) {
        self.bridgeSession = session
        self.config = config
        self.toolCallbacks = toolCallbacks
    }

    /// Returns the session history as a JSON string.
    ///
    /// - Returns: JSON string representation of the session transcript, or `nil` if encoding fails.
    func getHistoryJson() -> String? {
        do {
            let messages = convertTranscriptToMessages(bridgeSession.transcript)
            let jsonData = try JSONEncoder().encode(messages)
            return String(data: jsonData, encoding: .utf8)
        } catch {
            return nil
        }
    }

    /// Clears the session history.
    ///
    /// - Note: This function is kept for API compatibility but may be a no-op
    ///   if `LanguageModelSession` doesn't provide a public clear method.
    func clearHistory() {
        // No-op: LanguageModelSession may not have a public clear method
    }
}

@available(macOS 26.0, *)
private class SessionManager {
    static let shared = SessionManager()
    private var sessions: [UInt8: SessionInfo] = [:]
    private var streams: [UInt8: Task<Void, Never>] = [:]
    private var nextSessionId: UInt8 = 1
    private var nextStreamId: UInt8 = 1
    private let lock = NSLock()

    private init() {}

    /// Creates a new AI session with the specified configuration.
    ///
    /// - Parameters:
    ///   - model: The system language model to use.
    ///   - guardrails: Safety guardrails for content filtering.
    ///   - instructions: Optional system instructions for the AI.
    ///   - toolDefinitions: Array of tool definitions for function calling.
    ///   - config: Session configuration parameters.
    ///   - prewarm: Whether to prewarm the session for faster first response.
    /// - Returns: Unique session identifier.
    /// - Throws: `AIBridgeError` if session creation fails.
    func createSession(
        instructions: String?,
        toolDefinitions: [ClaudeToolDefinition],
        config: SessionConfig,
        prewarm: Bool
    ) throws -> UInt8 {
        lock.lock()
        defer { lock.unlock() }

        let sessionId = nextSessionId
        nextSessionId = nextSessionId &+ 1

        var bridgeTools: [any Tool] = []
        let toolCallbacks: [String: SessionInfo.ToolCallback] = [:]

        // Use concrete tool implementations for known tools
        // This gives the model properly typed @Generable arguments
        for toolDef in toolDefinitions {
            switch toolDef.name {
            case "web_search":
                bridgeTools.append(WebSearchTool(sessionId: sessionId))
            case "get_time":
                bridgeTools.append(GetTimeTool(sessionId: sessionId))
            case "past_sessions":
                bridgeTools.append(PastSessionsTool(sessionId: sessionId))
            case "read_document":
                bridgeTools.append(ReadDocumentTool(sessionId: sessionId))
            default:
                // Fall back to generic BridgeTool for unknown tools
                let bridgeTool = BridgeTool(sessionId: sessionId, definition: toolDef)
                bridgeTools.append(bridgeTool)
            }
        }

        let session: LanguageModelSession
        if let instructions = instructions {
            if bridgeTools.isEmpty {
                session = LanguageModelSession(instructions: instructions)
            } else {
                session = LanguageModelSession(tools: bridgeTools, instructions: instructions)
            }
        } else {
            if bridgeTools.isEmpty {
                session = LanguageModelSession()
            } else {
                session = LanguageModelSession(tools: bridgeTools)
            }
        }

        let sessionInfo = SessionInfo(
            session: session, config: config, toolCallbacks: toolCallbacks)
        sessions[sessionId] = sessionInfo

        if prewarm {
            session.prewarm()
        }

        return sessionId
    }

    /// Retrieves session information for the given session ID.
    ///
    /// - Parameter sessionId: The session identifier.
    /// - Returns: Session information, or `nil` if session doesn't exist.
    func getSession(_ sessionId: UInt8) -> SessionInfo? {
        lock.lock()
        defer { lock.unlock() }
        return sessions[sessionId]
    }

    /// Registers a tool callback for the specified session and tool.
    ///
    /// - Parameters:
    ///   - sessionId: The session identifier.
    ///   - toolName: The name of the tool.
    ///   - callback: C function pointer to handle tool execution.
    ///   - userData: Optional user data passed to the callback.
    func registerToolCallback(
        sessionId: UInt8,
        toolName: String,
        callback: @escaping @convention(c) (UnsafePointer<CChar>, UnsafeRawPointer?) ->
            UnsafeMutablePointer<CChar>?,
        userData: UnsafeRawPointer?
    ) {
        lock.lock()
        defer { lock.unlock() }

        if let sessionInfo = sessions[sessionId] {
            let toolCallback = SessionInfo.ToolCallback(callback: callback, userData: userData)
            sessionInfo.toolCallbacks[toolName] = toolCallback
        }
    }

    /// Retrieves the tool callback for the specified session and tool.
    ///
    /// - Parameters:
    ///   - sessionId: The session identifier.
    ///   - toolName: The name of the tool.
    /// - Returns: Tool callback information, or `nil` if not found.
    func getToolCallback(sessionId: UInt8, toolName: String) -> SessionInfo.ToolCallback? {
        lock.lock()
        defer { lock.unlock() }
        return sessions[sessionId]?.toolCallbacks[toolName]
    }

    /// Destroys the specified session and releases its resources.
    ///
    /// - Parameter sessionId: The session identifier to destroy.
    func destroySession(_ sessionId: UInt8) {
        lock.lock()
        defer { lock.unlock() }
        sessions.removeValue(forKey: sessionId)
    }

    /// Creates a new stream task and returns its identifier.
    ///
    /// - Parameter task: The async task to manage.
    /// - Returns: Unique stream identifier.
    func createStream(_ task: Task<Void, Never>) -> UInt8 {
        lock.lock()
        defer { lock.unlock() }

        let streamId = nextStreamId
        nextStreamId = nextStreamId &+ 1
        streams[streamId] = task
        return streamId
    }

    /// Cancels the specified stream.
    ///
    /// - Parameter streamId: The stream identifier to cancel.
    /// - Returns: `true` if the stream was found and cancelled, `false` otherwise.
    func cancelStream(_ streamId: UInt8) -> Bool {
        lock.lock()
        defer { lock.unlock() }

        if let task = streams.removeValue(forKey: streamId) {
            task.cancel()
            return true
        }
        return false
    }

    /// Removes the specified stream from management.
    ///
    /// - Parameter streamId: The stream identifier to remove.
    func removeStream(_ streamId: UInt8) {
        lock.lock()
        defer { lock.unlock() }
        streams.removeValue(forKey: streamId)
    }
}

// MARK: - Tool Support

@available(macOS 26.0, *)
private struct ClaudeToolDefinition: Codable {
    let name: String
    let description: String?
    let input_schema: [String: Any]?

    enum CodingKeys: String, CodingKey {
        case name, description, input_schema
    }

    init(from decoder: Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        name = try container.decode(String.self, forKey: .name)
        description = try container.decodeIfPresent(String.self, forKey: .description)

        if container.contains(.input_schema) {
            let schemaValue = try container.decode(AnyCodable.self, forKey: .input_schema)
            input_schema = schemaValue.value as? [String: Any]
        } else {
            input_schema = nil
        }
    }

    func encode(to encoder: Encoder) throws {
        var container = encoder.container(keyedBy: CodingKeys.self)
        try container.encode(name, forKey: .name)
        try container.encodeIfPresent(description, forKey: .description)

        if let schema = input_schema {
            try container.encode(AnyCodable(schema), forKey: .input_schema)
        }
    }
}

// MARK: - Concrete Tool Implementations
// Apple's FoundationModels requires @Generable structs with explicit properties.
// Generic JSON-string approach doesn't work well - the model needs to see actual parameters.

@available(macOS 26.0, *)
private struct WebSearchTool: Tool {
    var name: String = "web_search"
    var description: String = "Search the web for current information about facts, news, people, places, or events. Use this whenever the user asks about something you don't know or need to verify."
    private let sessionId: UInt8

    init(sessionId: UInt8) {
        self.sessionId = sessionId
    }

    @Generable
    struct Arguments {
        @Guide(description: "The search query - what to search for on the web")
        var query: String
    }

    func call(arguments: Arguments) async throws -> String {
        let jsonArgs = "{\"query\": \"\(arguments.query.replacingOccurrences(of: "\"", with: "\\\""))\"}"
        return try await executeCallback(sessionId: sessionId, toolName: "web_search", jsonArgs: jsonArgs)
    }
}

@available(macOS 26.0, *)
private struct GetTimeTool: Tool {
    var name: String = "get_time"
    var description: String = "Get the current date and time. Use this when the user asks what time or date it is."
    private let sessionId: UInt8

    init(sessionId: UInt8) {
        self.sessionId = sessionId
    }

    @Generable
    struct Arguments {
        // Empty - no arguments needed
    }

    func call(arguments: Arguments) async throws -> String {
        return try await executeCallback(sessionId: sessionId, toolName: "get_time", jsonArgs: "{}")
    }
}

@available(macOS 26.0, *)
private struct PastSessionsTool: Tool {
    var name: String = "past_sessions"
    var description: String = "Access the user's previous writing sessions. Use 'list' action to see all sessions, or 'read' action with a filename to read a specific session."
    private let sessionId: UInt8

    init(sessionId: UInt8) {
        self.sessionId = sessionId
    }

    @Generable
    struct Arguments {
        @Guide(description: "The action to perform: 'list' to see all sessions, or 'read' to read a specific session")
        var action: String

        @Guide(description: "The filename to read (only needed when action is 'read')")
        var filename: String?
    }

    func call(arguments: Arguments) async throws -> String {
        var jsonArgs = "{\"action\": \"\(arguments.action)\""
        if let filename = arguments.filename {
            jsonArgs += ", \"filename\": \"\(filename.replacingOccurrences(of: "\"", with: "\\\""))\""
        }
        jsonArgs += "}"
        return try await executeCallback(sessionId: sessionId, toolName: "past_sessions", jsonArgs: jsonArgs)
    }
}

@available(macOS 26.0, *)
private struct ReadDocumentTool: Tool {
    var name: String = "read_document"
    var description: String = "Read the user's current document in the editor. Use 'full' for entire document, 'selection' for selected text, 'info' for document stats (length, selection range, cursor position), or 'range' for text at a specific offset/length."
    private let sessionId: UInt8

    init(sessionId: UInt8) {
        self.sessionId = sessionId
    }

    @Generable
    struct Arguments {
        @Guide(description: "The action to perform: 'full' returns entire document (default), 'selection' returns selected text, 'info' returns document stats, 'range' returns text at offset/length")
        var action: String?

        @Guide(description: "Starting position in the document (only used with 'range' action)")
        var offset: Int?

        @Guide(description: "Number of characters to read (only used with 'range' action)")
        var length: Int?
    }

    func call(arguments: Arguments) async throws -> String {
        var jsonArgs = "{"
        var first = true

        if let action = arguments.action {
            jsonArgs += "\"action\": \"\(action)\""
            first = false
        }
        if let offset = arguments.offset {
            if !first { jsonArgs += ", " }
            jsonArgs += "\"offset\": \(offset)"
            first = false
        }
        if let length = arguments.length {
            if !first { jsonArgs += ", " }
            jsonArgs += "\"length\": \(length)"
        }
        jsonArgs += "}"

        return try await executeCallback(sessionId: sessionId, toolName: "read_document", jsonArgs: jsonArgs)
    }
}

// Helper to execute the registered C callback
@available(macOS 26.0, *)
private func executeCallback(sessionId: UInt8, toolName: String, jsonArgs: String) async throws -> String {
    guard let toolCallback = SessionManager.shared.getToolCallback(sessionId: sessionId, toolName: toolName) else {
        throw AIBridgeError.toolNotFound("Tool '\(toolName)' callback not registered")
    }

    let result = jsonArgs.withCString { cString in
        toolCallback.callback(cString, toolCallback.userData)
    }

    if let result = result {
        let resultString = String(cString: result)
        free(result)
        return resultString
    } else {
        throw AIBridgeError.toolExecutionError("Tool '\(toolName)' returned null")
    }
}

// Legacy BridgeTool for any tools not explicitly defined above
@available(macOS 26.0, *)
private struct BridgeTool: Tool {
    var name: String
    var description: String
    private let sessionId: UInt8
    private let toolName: String

    init(sessionId: UInt8, definition: ClaudeToolDefinition) {
        self.sessionId = sessionId
        self.name = definition.name
        self.toolName = definition.name
        self.description = definition.description ?? "Tool: \(definition.name)"
    }

    @Generable
    struct Arguments {
        @Guide(description: "JSON string containing the tool arguments as key-value pairs")
        var argumentsJson: String
    }

    func call(arguments: Arguments) async throws -> String {
        return try await executeCallback(sessionId: sessionId, toolName: toolName, jsonArgs: arguments.argumentsJson)
    }
}

// MARK: - Core Library Functions

/// Initializes the AI Bridge library.
///
/// - Returns: `true` if initialization was successful, `false` otherwise.
@available(macOS 26.0, *)
@_cdecl("ai_bridge_init")
public func bridgeInit() -> Bool {
    return true
}

// MARK: - Model Availability Functions

/// Checks the availability status of Apple Intelligence models.
///
/// - Returns: An `AIAvailabilityStatus` raw value indicating model availability.
@available(macOS 26.0, *)
@_cdecl("ai_bridge_check_availability")
public func bridgeCheckAvailability() -> Int32 {
    let model = SystemLanguageModel.default
    let availability = model.availability

    switch availability {
    case .available:
        return AIAvailabilityStatus.available.rawValue
    case .unavailable(let reason):
        switch reason {
        case .deviceNotEligible:
            return AIAvailabilityStatus.deviceNotEligible.rawValue
        case .appleIntelligenceNotEnabled:
            return AIAvailabilityStatus.intelligenceNotEnabled.rawValue
        case .modelNotReady:
            return AIAvailabilityStatus.modelNotReady.rawValue
        @unknown default:
            return AIAvailabilityStatus.unknownError.rawValue
        }
    @unknown default:
        return AIAvailabilityStatus.unknownError.rawValue
    }
}

/// Returns a human-readable description of the current availability status.
///
/// - Returns: A C string describing the availability status. **Memory ownership**: Caller must call `ai_bridge_free_string` to release.
@available(macOS 26.0, *)
@_cdecl("ai_bridge_get_availability_reason")
public func bridgeGetAvailabilityReason() -> UnsafeMutablePointer<CChar>? {
    let model = SystemLanguageModel.default
    let availability = model.availability

    let reasonString: String
    switch availability {
    case .available:
        reasonString = "Apple Intelligence is available and ready"

    case .unavailable(let reason):
        switch reason {
        case .deviceNotEligible:
            reasonString =
                "Device not eligible for Apple Intelligence. Supported devices: iPhone 15 Pro/Pro Max or newer, iPad with M1 chip or newer, Mac with Apple Silicon"
        case .appleIntelligenceNotEnabled:
            reasonString =
                "Apple Intelligence not enabled. Enable it in Settings > Apple Intelligence & Siri"
        case .modelNotReady:
            reasonString =
                "AI model not ready. Models download automatically based on network status, battery level, and system load. Please wait and try again later"
        @unknown default:
            reasonString = "Unknown availability issue occurred"
        }

    @unknown default:
        reasonString = "Unknown availability status"
    }

    return strdup(reasonString)
}

// MARK: - Session Management Functions

/// Creates a new AI session with the specified configuration.
///
/// - Parameters:
///   - instructions: Optional system instructions for the AI. May be `NULL`.
///   - toolsJson: JSON string defining available tools. May be `NULL`.
///   - enableGuardrails: Whether to enable content safety guardrails.
///   - enableHistory: Whether to maintain conversation history.
///   - enableStructuredResponses: Whether to enable structured response generation.
///   - defaultSchemaJson: Default JSON schema for structured responses. May be `NULL`.
///   - prewarm: Whether to prewarm the session for faster first response.
/// - Returns: Session identifier (non-zero on success, 0 on failure).
@available(macOS 26.0, *)
@_cdecl("ai_bridge_create_session")
public func bridgeCreateSession(
    instructions: UnsafePointer<CChar>?,
    toolsJson: UnsafePointer<CChar>?,
    enableGuardrails: Bool,
    enableHistory: Bool,
    enableStructuredResponses: Bool,
    defaultSchemaJson: UnsafePointer<CChar>?,
    prewarm: Bool
) -> UInt8 {
    do {
        let model = SystemLanguageModel.default
        guard case .available = model.availability else {
            return 0
        }

        let instructionsString = instructions.map { String(cString: $0) }
        let config = SessionConfig()

        var toolDefinitions: [ClaudeToolDefinition] = []
        if let toolsJson = toolsJson {
            let toolsJsonString = String(cString: toolsJson)
            if let toolsData = toolsJsonString.data(using: .utf8) {
                toolDefinitions = try JSONDecoder().decode(
                    [ClaudeToolDefinition].self, from: toolsData)
            }
        }

        return try SessionManager.shared.createSession(
            instructions: instructionsString,
            toolDefinitions: toolDefinitions,
            config: config,
            prewarm: prewarm
        )
    } catch {
        return 0
    }
}

/// Registers a tool callback function for the specified session.
///
/// - Parameters:
///   - sessionId: The session identifier.
///   - toolName: The name of the tool to register.
///   - callback: C function pointer that will be called when the tool is invoked.
///     The callback receives JSON arguments and must return a JSON result string.
///     **Memory ownership**: The callback must return a string allocated with `malloc` or `strdup`.
///   - userData: Optional user data passed to the callback.
/// - Returns: `true` if registration was successful, `false` otherwise.
@available(macOS 26.0, *)
@_cdecl("ai_bridge_register_tool")
public func bridgeRegisterTool(
    sessionId: UInt8,
    toolName: UnsafePointer<CChar>,
    callback: @escaping @convention(c) (UnsafePointer<CChar>, UnsafeRawPointer?) ->
        UnsafeMutablePointer<CChar>?,
    userData: UnsafeRawPointer?
) -> Bool {
    guard SessionManager.shared.getSession(sessionId) != nil else {
        return false
    }

    let toolNameString = String(cString: toolName)
    SessionManager.shared.registerToolCallback(
        sessionId: sessionId,
        toolName: toolNameString,
        callback: callback,
        userData: userData
    )

    return true
}

/// Destroys the specified session and releases all associated resources.
///
/// - Parameter sessionId: The session identifier to destroy.
@available(macOS 26.0, *)
@_cdecl("ai_bridge_destroy_session")
public func bridgeDestroySession(sessionId: UInt8) {
    SessionManager.shared.destroySession(sessionId)
}

// MARK: - History Management Functions

/// Retrieves the conversation history for the specified session.
///
/// - Parameter sessionId: The session identifier.
/// - Returns: JSON string containing the conversation history, or `NULL` if session not found or encoding fails.
///   **Memory ownership**: Caller must call `ai_bridge_free_string` to release.
@available(macOS 26.0, *)
@_cdecl("ai_bridge_get_session_history")
public func bridgeGetSessionHistory(sessionId: UInt8) -> UnsafeMutablePointer<CChar>? {
    guard let sessionInfo = SessionManager.shared.getSession(sessionId) else {
        return nil
    }

    guard let historyJson = sessionInfo.getHistoryJson() else {
        return nil
    }

    return strdup(historyJson)
}

/// Clears the conversation history for the specified session.
///
/// - Parameter sessionId: The session identifier.
/// - Returns: `true` if the operation was successful, `false` if session not found.
/// - Note: This function may be a no-op depending on the underlying session implementation.
@available(macOS 26.0, *)
@_cdecl("ai_bridge_clear_session_history")
public func bridgeClearSessionHistory(sessionId: UInt8) -> Bool {
    guard let sessionInfo = SessionManager.shared.getSession(sessionId) else {
        return false
    }

    sessionInfo.clearHistory()
    return true
}

/// Adds a message to the session history.
///
/// - Parameters:
///   - sessionId: The session identifier.
///   - role: The role of the message sender (e.g., "user", "assistant").
///   - content: The message content.
/// - Returns: `true` if the operation was successful, `false` if session not found.
/// - Note: This function is kept for API compatibility but is now a no-op
///   since the session automatically manages its transcript.
@available(macOS 26.0, *)
@_cdecl("ai_bridge_add_message_to_history")
public func bridgeAddMessageToHistory(
    sessionId: UInt8,
    role: UnsafePointer<CChar>,
    content: UnsafePointer<CChar>
) -> Bool {
    guard SessionManager.shared.getSession(sessionId) != nil else {
        return false
    }

    return true
}

// MARK: - Text Generation Functions (Synchronous)

/// Generates a text response for the given prompt.
///
/// - Parameters:
///   - sessionId: The session identifier.
///   - prompt: The input prompt text.
///   - temperature: Controls randomness in generation (0.0 = deterministic, 1.0 = very random).
///   - maxTokens: Maximum number of tokens to generate (0 = no limit).
/// - Returns: Generated response text, or error message if generation fails.
///   **Memory ownership**: Caller must call `ai_bridge_free_string` to release.
@available(macOS 26.0, *)
@_cdecl("ai_bridge_generate_response")
public func bridgeGenerateResponse(
    sessionId: UInt8,
    prompt: UnsafePointer<CChar>,
    temperature: Double,
    maxTokens: Int32
) -> UnsafeMutablePointer<CChar>? {
    let promptString = String(cString: prompt)

    return performSynchronousTask {
        try await generateResponse(
            sessionId: sessionId,
            prompt: promptString,
            temperature: temperature,
            maxTokens: maxTokens
        )
    }
}

/// Generates a structured response conforming to the provided JSON schema.
///
/// - Parameters:
///   - sessionId: The session identifier.
///   - prompt: The input prompt text.
///   - schemaJson: JSON schema defining the expected response structure. May be `NULL`.
///   - temperature: Controls randomness in generation (0.0 = deterministic, 1.0 = very random).
///   - maxTokens: Maximum number of tokens to generate (0 = no limit).
/// - Returns: JSON string containing both text and structured object representations.
///   **Memory ownership**: Caller must call `ai_bridge_free_string` to release.
@available(macOS 26.0, *)
@_cdecl("ai_bridge_generate_structured_response")
public func bridgeGenerateStructuredResponse(
    sessionId: UInt8,
    prompt: UnsafePointer<CChar>,
    schemaJson: UnsafePointer<CChar>?,
    temperature: Double,
    maxTokens: Int32
) -> UnsafeMutablePointer<CChar>? {
    let promptString = String(cString: prompt)
    let schemaJsonString = schemaJson.map { String(cString: $0) }

    return performSynchronousTask {
        try await generateStructuredResponse(
            sessionId: sessionId,
            prompt: promptString,
            schemaJson: schemaJsonString,
            temperature: temperature,
            maxTokens: maxTokens
        )
    }
}

// MARK: - Streaming Functions

/// Starts streaming text generation for the given prompt.
///
/// - Parameters:
///   - sessionId: The session identifier.
///   - prompt: The input prompt text.
///   - temperature: Controls randomness in generation (0.0 = deterministic, 1.0 = very random).
///   - maxTokens: Maximum number of tokens to generate (0 = no limit).
///   - context: Opaque pointer passed to the callback.
///   - callback: Function called for each token or error. Receives context, token (or `NULL` for completion/error), and userData.
///   - userData: Optional user data passed to the callback.
/// - Returns: Stream identifier for cancellation (0 if failed to start).
@available(macOS 26.0, *)
@_cdecl("ai_bridge_generate_response_stream")
public func bridgeGenerateResponseStream(
    sessionId: UInt8,
    prompt: UnsafePointer<CChar>,
    temperature: Double,
    maxTokens: Int32,
    context: UnsafeRawPointer,
    callback: @escaping @convention(c) (UnsafeRawPointer, UnsafePointer<CChar>?, UnsafeRawPointer?)
        -> Void,
    userData: UnsafeRawPointer?
) -> UInt8 {
    let promptString = String(cString: prompt)

    let task = Task.detached {
        do {
            guard let sessionInfo = SessionManager.shared.getSession(sessionId) else {
                emitError(
                    "Session not found", context: context, callback: callback, userData: userData)
                return
            }

            let options = createGenerationOptions(temperature: temperature, maxTokens: maxTokens)
            let session = sessionInfo.bridgeSession

            var previousContent = ""

            for try await snapshot in session.streamResponse(
                to: promptString, options: options)
            {
                var cumulativeContent = snapshot.content

                // Skip "null" prefix that sometimes appears in first snapshot
                if previousContent.isEmpty && cumulativeContent == "null" {
                    continue
                }
                if previousContent.isEmpty && cumulativeContent.hasPrefix("null") {
                    cumulativeContent = String(cumulativeContent.dropFirst(4))
                    // Also skip any leading whitespace/newline after "null"
                    while cumulativeContent.hasPrefix(" ") || cumulativeContent.hasPrefix("\n") {
                        cumulativeContent = String(cumulativeContent.dropFirst())
                    }
                }

                guard cumulativeContent.count > previousContent.count else { continue }

                let deltaStartIndex = cumulativeContent.index(
                    cumulativeContent.startIndex,
                    offsetBy: previousContent.count
                )
                let deltaContent = String(cumulativeContent[deltaStartIndex...])
                previousContent = cumulativeContent

                guard !deltaContent.isEmpty else { continue }

                deltaContent.withCString { cString in
                    callback(context, cString, userData)
                }
            }

            callback(context, nil, userData)

        } catch LanguageModelSession.GenerationError.guardrailViolation {
            emitError(
                "Guardrail violation: Content blocked by safety filters", context: context,
                callback: callback, userData: userData)
        } catch {
            emitError(
                error.localizedDescription, context: context, callback: callback, userData: userData
            )
        }
    }

    return SessionManager.shared.createStream(task)
}

/// Starts streaming structured response generation for the given prompt.
///
/// - Parameters:
///   - sessionId: The session identifier.
///   - prompt: The input prompt text.
///   - schemaJson: JSON schema defining the expected response structure. May be `NULL`.
///   - temperature: Controls randomness in generation (0.0 = deterministic, 1.0 = very random).
///   - maxTokens: Maximum number of tokens to generate (0 = no limit).
///   - context: Opaque pointer passed to the callback.
///   - callback: Function called with the complete structured response or error. Receives context, JSON result (or `NULL` for error), and userData.
///   - userData: Optional user data passed to the callback.
/// - Returns: Stream identifier for cancellation (0 if failed to start).
@available(macOS 26.0, *)
@_cdecl("ai_bridge_generate_structured_response_stream")
public func bridgeGenerateStructuredResponseStream(
    sessionId: UInt8,
    prompt: UnsafePointer<CChar>,
    schemaJson: UnsafePointer<CChar>?,
    temperature: Double,
    maxTokens: Int32,
    context: UnsafeRawPointer,
    callback: @escaping @convention(c) (UnsafeRawPointer, UnsafePointer<CChar>?, UnsafeRawPointer?)
        -> Void,
    userData: UnsafeRawPointer?
) -> UInt8 {
    let promptString = String(cString: prompt)
    let schemaJsonString = schemaJson.map { String(cString: $0) }

    let task = Task.detached {
        do {
            guard let sessionInfo = SessionManager.shared.getSession(sessionId) else {
                emitError(
                    "Session not found", context: context, callback: callback, userData: userData)
                return
            }

            let finalSchemaJson: String
            if let providedSchema = schemaJsonString {
                finalSchemaJson = providedSchema
            } else {
                emitError(
                    "No schema provided and session not configured for structured responses",
                    context: context, callback: callback, userData: userData)
                return
            }

            guard let schemaData = finalSchemaJson.data(using: .utf8),
                let jsonObject = try JSONSerialization.jsonObject(with: schemaData)
                    as? [String: Any]
            else {
                emitError(
                    "Invalid JSON Schema", context: context, callback: callback, userData: userData)
                return
            }

            let (rootSchema, dependencies) = buildSchemasFromJSON(jsonObject)
            let generationSchema = try GenerationSchema(
                root: rootSchema, dependencies: dependencies)
            let options = createGenerationOptions(temperature: temperature, maxTokens: maxTokens)
            let session = sessionInfo.bridgeSession

            let response = try await session.respond(
                to: promptString,
                schema: generationSchema,
                includeSchemaInPrompt: true,
                options: options
            )

            let objectJSON = convertGeneratedContentToJSON(response.content)
            let textRepresentation = String(describing: response.content)

            let responseJSON: [String: Any] = [
                "text": textRepresentation,
                "object": objectJSON,
            ]

            let jsonData = try JSONSerialization.data(withJSONObject: responseJSON, options: [])
            guard let jsonString = String(data: jsonData, encoding: .utf8) else {
                emitError(
                    "Failed to encode response as JSON", context: context, callback: callback,
                    userData: userData)
                return
            }

            jsonString.withCString { cString in
                callback(context, cString, userData)
            }

            callback(context, nil, userData)

        } catch LanguageModelSession.GenerationError.guardrailViolation {
            emitError(
                "Guardrail violation: Content blocked by safety filters", context: context,
                callback: callback, userData: userData)
        } catch {
            emitError(
                error.localizedDescription, context: context, callback: callback, userData: userData
            )
        }
    }

    return SessionManager.shared.createStream(task)
}

// MARK: - Stream Control Functions

/// Cancels the specified stream.
///
/// - Parameter streamId: The stream identifier to cancel.
/// - Returns: `true` if the stream was found and cancelled, `false` otherwise.
@available(macOS 26.0, *)
@_cdecl("ai_bridge_cancel_stream")
public func bridgeCancelStream(streamId: UInt8) -> Bool {
    return SessionManager.shared.cancelStream(streamId)
}

// MARK: - Language Support Functions

/// Returns the number of supported languages.
///
/// - Returns: Count of supported languages.
@available(macOS 26.0, *)
@_cdecl("ai_bridge_get_supported_languages_count")
public func bridgeGetSupportedLanguagesCount() -> Int32 {
    let model = SystemLanguageModel.default
    return Int32(Array(model.supportedLanguages).count)
}

/// Returns the display name of the supported language at the given index.
///
/// - Parameter index: Zero-based index of the language.
/// - Returns: Display name of the language, or `NULL` if index is out of bounds.
///   **Memory ownership**: Caller must call `ai_bridge_free_string` to release.
@available(macOS 26.0, *)
@_cdecl("ai_bridge_get_supported_language")
public func bridgeGetSupportedLanguage(index: Int32) -> UnsafeMutablePointer<CChar>? {
    let model = SystemLanguageModel.default
    let languagesArray = Array(model.supportedLanguages)

    guard index >= 0 && index < Int32(languagesArray.count) else {
        return nil
    }

    let language = languagesArray[Int(index)]
    let locale = Locale(identifier: language.maximalIdentifier)

    if let displayName = locale.localizedString(forIdentifier: language.maximalIdentifier) {
        return strdup(displayName)
    }

    if let languageCode = language.languageCode?.identifier {
        return strdup(languageCode)
    }

    return strdup("Unknown Language")
}

// MARK: - Memory Management

/// Frees a string allocated by the AI Bridge library.
///
/// - Parameter ptr: Pointer to the string to free. May be `NULL`.
/// - Note: This function must be called for all strings returned by AI Bridge functions
///   to prevent memory leaks.
@available(macOS 26.0, *)
@_cdecl("ai_bridge_free_string")
public func bridgeFreeString(ptr: UnsafeMutablePointer<CChar>?) {
    if let ptr = ptr {
        free(ptr)
    }
}

// MARK: - Internal Implementation

@available(macOS 26.0, *)
private func generateResponse(
    sessionId: UInt8,
    prompt: String,
    temperature: Double,
    maxTokens: Int32
) async throws -> String {
    guard let sessionInfo = SessionManager.shared.getSession(sessionId) else {
        throw AIBridgeError.sessionNotFound
    }

    let options = createGenerationOptions(temperature: temperature, maxTokens: maxTokens)
    let session = sessionInfo.bridgeSession

    let response = try await session.respond(to: prompt, options: options)
    return response.content
}

@available(macOS 26.0, *)
private func generateStructuredResponse(
    sessionId: UInt8,
    prompt: String,
    schemaJson: String?,
    temperature: Double,
    maxTokens: Int32
) async throws -> String {
    guard let sessionInfo = SessionManager.shared.getSession(sessionId) else {
        throw AIBridgeError.sessionNotFound
    }

    let finalSchemaJson: String
    if let providedSchema = schemaJson {
        finalSchemaJson = providedSchema
    } else {
        throw AIBridgeError.invalidJSON(
            "No schema provided and session not configured for structured responses")
    }

    guard let schemaData = finalSchemaJson.data(using: .utf8),
        let jsonObject = try JSONSerialization.jsonObject(with: schemaData) as? [String: Any]
    else {
        throw AIBridgeError.invalidJSON("Invalid JSON Schema")
    }

    let (rootSchema, dependencies) = buildSchemasFromJSON(jsonObject)
    let generationSchema = try GenerationSchema(root: rootSchema, dependencies: dependencies)
    let options = createGenerationOptions(temperature: temperature, maxTokens: maxTokens)
    let session = sessionInfo.bridgeSession

    let response = try await session.respond(
        to: prompt,
        schema: generationSchema,
        includeSchemaInPrompt: true,
        options: options
    )

    let objectJSON = convertGeneratedContentToJSON(response.content)
    let textRepresentation = String(describing: response.content)

    let responseJSON: [String: Any] = [
        "text": textRepresentation,
        "object": objectJSON,
    ]

    let jsonData = try JSONSerialization.data(withJSONObject: responseJSON, options: [])
    guard let jsonString = String(data: jsonData, encoding: .utf8) else {
        throw AIBridgeError.encodingError("Failed to encode response as JSON")
    }

    return jsonString
}

// MARK: - Helper Functions

@available(macOS 26.0, *)
private func createGenerationOptions(temperature: Double, maxTokens: Int32) -> GenerationOptions {
    var options: GenerationOptions = GenerationOptions()

    if temperature > 0 {
        options.temperature = temperature
    }

    if maxTokens > 0 {
        options.maximumResponseTokens = Int(maxTokens)
    }

    return options
}

@available(macOS 26.0, *)
private func performSynchronousTask<T>(_ operation: @escaping () async throws -> T)
    -> UnsafeMutablePointer<CChar>?
{
    let semaphore = DispatchSemaphore(value: 0)
    var result: String = "Error: No response generated"

    Task {
        do {
            let value = try await operation()
            if let stringValue = value as? String {
                result = stringValue
            } else {
                result = String(describing: value)
            }
        } catch LanguageModelSession.GenerationError.guardrailViolation {
            result = "Error: Guardrail violation - Content blocked by safety filters"
        } catch AIBridgeError.sessionNotFound {
            result = "Error: Session not found"
        } catch AIBridgeError.toolNotFound(let message) {
            result = "Error: \(message)"
        } catch {
            result = "Error: \(error.localizedDescription)"
        }
        semaphore.signal()
    }

    semaphore.wait()
    return strdup(result)
}

@available(macOS 26.0, *)
private func emitError(
    _ message: String,
    context: UnsafeRawPointer,
    callback: @escaping @convention(c) (UnsafeRawPointer, UnsafePointer<CChar>?, UnsafeRawPointer?)
        -> Void,
    userData: UnsafeRawPointer?
) {
    let errorMessage = "Error: \(message)"
    errorMessage.withCString { cString in
        callback(context, cString, userData)
    }
}

// MARK: - Transcript Conversion Helper

@available(macOS 26.0, *)
private func convertTranscriptToMessages(_ transcript: Transcript) -> [ChatMessage] {
    var messages: [ChatMessage] = []

    for entry in transcript {
        switch entry {
        case .instructions(let instructions):
            let content = extractTextFromSegments(instructions.segments)
            if !content.isEmpty {
                messages.append(ChatMessage(role: "system", content: content))
            }

        case .prompt(let prompt):
            let content = extractTextFromSegments(prompt.segments)
            if !content.isEmpty {
                messages.append(ChatMessage(role: "user", content: content))
            }

        case .response(let response):
            let content = extractTextFromSegments(response.segments)
            if !content.isEmpty {
                messages.append(ChatMessage(role: "assistant", content: content))
            }

        case .toolCalls(let toolCalls):
            var chatToolCalls: [ChatMessage.ToolCall] = []
            for toolCall in toolCalls {
                let argumentsJson = convertGeneratedContentToJSONString(toolCall.arguments)
                let chatToolCall = ChatMessage.ToolCall(
                    id: toolCall.id,
                    type: "function",
                    function: ChatMessage.ToolCall.Function(
                        name: toolCall.toolName,
                        arguments: argumentsJson
                    )
                )
                chatToolCalls.append(chatToolCall)
            }

            if !chatToolCalls.isEmpty {
                messages.append(
                    ChatMessage(
                        role: "assistant",
                        content: "",
                        toolCalls: chatToolCalls
                    ))
            }

        case .toolOutput(let toolOutput):
            let content = extractTextFromSegments(toolOutput.segments)
            messages.append(
                ChatMessage(
                    role: "tool",
                    content: content,
                    toolCallId: toolOutput.id,
                    toolName: toolOutput.toolName
                ))

        @unknown default:
            messages.append(
                ChatMessage(
                    role: "unknown",
                    content: String(describing: entry)
                ))
        }
    }

    return messages
}

@available(macOS 26.0, *)
private func extractTextFromSegments(_ segments: [Transcript.Segment]) -> String {
    var textContent: [String] = []

    for segment in segments {
        switch segment {
        case .text(let textSegment):
            textContent.append(textSegment.content)
        case .structure(let structuredSegment):
            let jsonObject = convertGeneratedContentToJSON(structuredSegment.content)
            if let jsonData = try? JSONSerialization.data(
                withJSONObject: jsonObject, options: [.prettyPrinted]),
                let jsonString = String(data: jsonData, encoding: .utf8)
            {
                textContent.append(jsonString)
            } else {
                textContent.append(String(describing: structuredSegment.content))
            }
        default:
            textContent.append(String(describing: segment))
        }
    }

    return textContent.joined(separator: "\n")
}

@available(macOS 26.0, *)
private func convertGeneratedContentToJSONString(_ content: GeneratedContent) -> String {
    let jsonObject = convertGeneratedContentToJSON(content)
    if let jsonData = try? JSONSerialization.data(withJSONObject: jsonObject, options: []),
        let jsonString = String(data: jsonData, encoding: .utf8)
    {
        return jsonString
    }
    return "{}"
}

// MARK: - Data Models and Helper Functions

@available(macOS 26.0, *)
private struct ChatMessage: Codable {
    let role: String
    let content: String
    let name: String?
    let toolCalls: [ToolCall]?
    let toolCallId: String?
    let toolName: String?

    init(
        role: String,
        content: String,
        name: String? = nil,
        toolCalls: [ToolCall]? = nil,
        toolCallId: String? = nil,
        toolName: String? = nil
    ) {
        self.role = role
        self.content = content
        self.name = name
        self.toolCalls = toolCalls
        self.toolCallId = toolCallId
        self.toolName = toolName
    }

    struct ToolCall: Codable {
        let id: String
        let type: String
        let function: Function

        struct Function: Codable {
            let name: String
            let arguments: String
        }
    }
}

@available(macOS 26.0, *)
private struct AnyCodable: Codable {
    let value: Any

    init(_ value: Any) {
        self.value = value
    }

    init(from decoder: Decoder) throws {
        let container = try decoder.singleValueContainer()

        if let bool = try? container.decode(Bool.self) {
            value = bool
        } else if let int = try? container.decode(Int.self) {
            value = int
        } else if let double = try? container.decode(Double.self) {
            value = double
        } else if let string = try? container.decode(String.self) {
            value = string
        } else if let array = try? container.decode([AnyCodable].self) {
            value = array.map { $0.value }
        } else if let dict = try? container.decode([String: AnyCodable].self) {
            value = dict.mapValues { $0.value }
        } else {
            value = NSNull()
        }
    }

    func encode(to encoder: Encoder) throws {
        var container = encoder.singleValueContainer()

        switch value {
        case let bool as Bool:
            try container.encode(bool)
        case let int as Int:
            try container.encode(int)
        case let double as Double:
            try container.encode(double)
        case let string as String:
            try container.encode(string)
        case let array as [Any]:
            try container.encode(array.map { AnyCodable($0) })
        case let dict as [String: Any]:
            try container.encode(dict.mapValues { AnyCodable($0) })
        default:
            try container.encodeNil()
        }
    }
}

@available(macOS 26.0, *)
private func buildSchemasFromJSON(_ json: [String: Any]) -> (
    DynamicGenerationSchema, [DynamicGenerationSchema]
) {
    var dependencies: [DynamicGenerationSchema] = []

    var rootNameFromRef: String?
    if let ref = json["$ref"] as? String, ref.hasPrefix("#/definitions/") {
        rootNameFromRef = String(ref.dropFirst("#/definitions/".count))
    }

    if let definitions = json["definitions"] as? [String: Any] {
        for (name, definitionValue) in definitions {
            guard let definitionDict = definitionValue as? [String: Any] else { continue }

            if let rootName = rootNameFromRef, name == rootName { continue }

            let dependencySchema = convertJSONSchemaToDynamicSchema(definitionDict, name: name)
            dependencies.append(dependencySchema)
        }
    }

    if let rootName = rootNameFromRef,
        let definitions = json["definitions"] as? [String: Any],
        let rootDefinition = definitions[rootName] as? [String: Any]
    {
        let rootSchema = convertJSONSchemaToDynamicSchema(rootDefinition, name: rootName)
        return (rootSchema, dependencies)
    }

    let rootSchema = convertJSONSchemaToDynamicSchema(json, name: json["title"] as? String)
    return (rootSchema, dependencies)
}

@available(macOS 26.0, *)
private func convertJSONSchemaToDynamicSchema(_ dict: [String: Any], name: String? = nil)
    -> DynamicGenerationSchema
{
    if let ref = dict["$ref"] as? String {
        return DynamicGenerationSchema(referenceTo: ref)
    }

    if let anyOf = dict["anyOf"] as? [[String: Any]] {
        var stringChoices: [String] = []
        var schemaChoices: [DynamicGenerationSchema] = []

        for choice in anyOf {
            if let enums = choice["enum"] as? [String], enums.count == 1 {
                stringChoices.append(enums[0])
            } else {
                schemaChoices.append(convertJSONSchemaToDynamicSchema(choice))
            }
        }

        if !stringChoices.isEmpty && schemaChoices.isEmpty {
            return DynamicGenerationSchema(
                name: name ?? UUID().uuidString,
                description: dict["description"] as? String,
                anyOf: stringChoices
            )
        } else {
            let choices =
                schemaChoices.isEmpty
                ? anyOf.map { convertJSONSchemaToDynamicSchema($0) } : schemaChoices
            return DynamicGenerationSchema(
                name: name ?? UUID().uuidString,
                description: dict["description"] as? String,
                anyOf: choices
            )
        }
    }

    if let enums = dict["enum"] as? [String] {
        return DynamicGenerationSchema(
            name: name ?? UUID().uuidString,
            description: dict["description"] as? String,
            anyOf: enums
        )
    }

    guard let type = dict["type"] as? String else {
        return DynamicGenerationSchema(type: String.self)
    }

    switch type {
    case "string":
        return DynamicGenerationSchema(type: String.self)
    case "number":
        return DynamicGenerationSchema(type: Double.self)
    case "integer":
        return DynamicGenerationSchema(type: Int.self)
    case "boolean":
        return DynamicGenerationSchema(type: Bool.self)
    case "array":
        let itemSchema: DynamicGenerationSchema
        if let items = dict["items"] as? [String: Any] {
            itemSchema = convertJSONSchemaToDynamicSchema(items)
        } else {
            itemSchema = DynamicGenerationSchema(type: String.self)
        }

        let minItems = dict["minItems"] as? Int
        let maxItems = dict["maxItems"] as? Int

        return DynamicGenerationSchema(
            arrayOf: itemSchema,
            minimumElements: minItems,
            maximumElements: maxItems
        )

    case "object":
        let requiredFields = (dict["required"] as? [String]) ?? []
        var properties: [DynamicGenerationSchema.Property] = []

        if let propertiesDict = dict["properties"] as? [String: Any] {
            for (propertyName, propertyValue) in propertiesDict {
                guard let propertyDict = propertyValue as? [String: Any] else { continue }

                let propertySchema = convertJSONSchemaToDynamicSchema(
                    propertyDict, name: propertyName)
                let isOptional = !requiredFields.contains(propertyName)

                let property = DynamicGenerationSchema.Property(
                    name: propertyName,
                    description: propertyDict["description"] as? String,
                    schema: propertySchema,
                    isOptional: isOptional
                )
                properties.append(property)
            }
        }

        return DynamicGenerationSchema(
            name: name ?? "Object",
            description: dict["description"] as? String,
            properties: properties
        )

    default:
        return DynamicGenerationSchema(type: String.self)
    }
}

@available(macOS 26.0, *)
private func convertGeneratedContentToJSON(_ content: GeneratedContent) -> Any {
    // Try to extract primitive values
    if let str = try? content.value(String.self) { return str }
    if let intVal = try? content.value(Int.self) { return intVal }
    if let dbl = try? content.value(Double.self) { return dbl }
    if let boolVal = try? content.value(Bool.self) { return boolVal }

    // Fallback to string representation
    return String(describing: content)
}

@available(macOS 26.0, *)
private func convertJSONToGeneratedContent(_ jsonObject: Any) -> GeneratedContent {
    switch jsonObject {
    case let dict as [String: Any]:
        let sortedKeys = dict.keys.sorted()
        var keyValuePairs: [(String, any ConvertibleToGeneratedContent)] = []

        for key in sortedKeys {
            let value = convertJSONToGeneratedContent(dict[key]!)
            keyValuePairs.append((key, value))
        }

        return buildGeneratedContentFromPairs(keyValuePairs)

    case let array as [Any]:
        let elements = array.map { convertJSONToGeneratedContent($0) }
        return GeneratedContent(elements: elements)

    case let string as String:
        return GeneratedContent(string as String)

    case let number as NSNumber:
        if number === kCFBooleanTrue || number === kCFBooleanFalse {
            return GeneratedContent(number.boolValue as Bool)
        }

        let objCType = String(cString: number.objCType)
        if objCType == "d" || objCType == "f" {
            return GeneratedContent(number.doubleValue as Double)
        } else {
            return GeneratedContent(number.intValue as Int)
        }

    case let bool as Bool:
        return GeneratedContent(bool as Bool)

    case let int as Int:
        return GeneratedContent(int as Int)

    case let double as Double:
        return GeneratedContent(double as Double)

    case let float as Float:
        return GeneratedContent(Double(float) as Double)

    default:
        return GeneratedContent("" as String)
    }
}

@available(macOS 26.0, *)
private func buildGeneratedContentFromPairs(_ pairs: [(String, any ConvertibleToGeneratedContent)])
    -> GeneratedContent
{
    switch pairs.count {
    case 0:
        return GeneratedContent(properties: [:])
    case 1:
        return GeneratedContent(properties: [pairs[0].0: pairs[0].1])
    case 2:
        return GeneratedContent(properties: [pairs[0].0: pairs[0].1, pairs[1].0: pairs[1].1])
    case 3:
        return GeneratedContent(properties: [
            pairs[0].0: pairs[0].1, pairs[1].0: pairs[1].1, pairs[2].0: pairs[2].1,
        ])
    case 4:
        return GeneratedContent(properties: [
            pairs[0].0: pairs[0].1, pairs[1].0: pairs[1].1, pairs[2].0: pairs[2].1,
            pairs[3].0: pairs[3].1,
        ])
    case 5:
        return GeneratedContent(properties: [
            pairs[0].0: pairs[0].1, pairs[1].0: pairs[1].1, pairs[2].0: pairs[2].1,
            pairs[3].0: pairs[3].1, pairs[4].0: pairs[4].1,
        ])
    default:
        return GeneratedContent(properties: [
            pairs[0].0: pairs[0].1, pairs[1].0: pairs[1].1, pairs[2].0: pairs[2].1,
            pairs[3].0: pairs[3].1, pairs[4].0: pairs[4].1,
        ])
    }
}

@available(macOS 26.0, *)
private enum AIBridgeError: LocalizedError {
    case modelUnavailable
    case invalidJSON(String)
    case invalidInput(String)
    case encodingError(String)
    case sessionNotFound
    case streamNotFound
    case guardrailViolation
    case toolExecutionError(String)
    case toolNotFound(String)

    var errorDescription: String? {
        switch self {
        case .modelUnavailable:
            return "Apple Intelligence model is not available"
        case .invalidJSON(let message):
            return "Invalid JSON: \(message)"
        case .invalidInput(let message):
            return "Invalid input: \(message)"
        case .encodingError(let message):
            return "Encoding error: \(message)"
        case .sessionNotFound:
            return "Session not found"
        case .streamNotFound:
            return "Stream not found"
        case .guardrailViolation:
            return "Content blocked by safety filters"
        case .toolExecutionError(let message):
            return "Tool execution error: \(message)"
        case .toolNotFound(let message):
            return "Tool not found: \(message)"
        }
    }
}