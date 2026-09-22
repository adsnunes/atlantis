//! @file lang/csharp.c
//! @brief C# language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    // XML documentation comments
    { .pattern = "///.*(?:\\n|$)", .token = HL_TOKEN_CMNT },
    // Regular comments
    { .pattern = "//.*(?:\\n|$)|/\\*(?:(?!\\*/).|[\\s\\S])*(?:\\*/)?", .token = HL_TOKEN_CMNT },
    // Verbatim strings
    { .pattern = "@\"(?:[^\"]|\"\")*\"?", .token = HL_TOKEN_STR },
    // Interpolated strings
    { .pattern = "\\$\"(?:[^\"\\\\]|\\\\[\\s\\S])*\"?", .token = HL_TOKEN_STR },
    { .pattern = "\\$@\"(?:[^\"]|\"\")*\"?", .token = HL_TOKEN_STR },
    // Regular strings and chars
    { .pattern = "([\"'])(?:\\\\[\\s\\S]|(?!\\1)[^\\r\\n\\\\])*\\1?", .token = HL_TOKEN_STR },
    // Numbers (including suffixes)
    { .pattern = "(?:\\.e?|\\b)\\d(?:e-|[\\d.oxa-fA-F_])*[dDfFmMuUlL]*(?:\\.|\\b)", .token = HL_TOKEN_NUM },
    // Attributes
    { .pattern = "\\[[A-Za-z_][\\w]*(?:\\([^)]*\\))?\\]", .token = HL_TOKEN_FUNC },
    // Preprocessor directives
    { .pattern = "^\\s*#(?:if|else|elif|endif|define|undef|warning|error|line|region|endregion|pragma|nullable)\\b.*", .token = HL_TOKEN_SECTION, .flags = HL_RULE_MULTILINE },
    // Keywords
    { .pattern = "\\b(?:abstract|as|base|break|case|catch|checked|class|const|continue|default|delegate|do|else|enum|event|explicit|extern|finally|fixed|for|foreach|goto|if|implicit|in|interface|internal|is|lock|namespace|new|operator|out|override|params|partial|private|protected|public|readonly|ref|return|sealed|sizeof|stackalloc|static|struct|switch|this|throw|try|typeof|unchecked|unsafe|using|virtual|volatile|while|yield|async|await|var|dynamic|nameof|when|record|init|with|required|file|scoped)\\b", .token = HL_TOKEN_KWD },
    // Primitive types
    { .pattern = "\\b(?:bool|byte|sbyte|char|decimal|double|float|int|uint|long|ulong|short|ushort|object|string|void|nint|nuint)\\b", .token = HL_TOKEN_TYPE },
    // Common .NET types
    { .pattern = "\\b(?:String|Int32|Int64|Double|Single|Boolean|Byte|Char|Decimal|Object|Type|Void|Array|Enum|Delegate|Exception|Attribute)\\b", .token = HL_TOKEN_TYPE },
    { .pattern = "\\b(?:List|Dictionary|HashSet|Queue|Stack|LinkedList|SortedList|SortedSet|SortedDictionary|IEnumerable|ICollection|IList|IDictionary|ISet)\\b", .token = HL_TOKEN_TYPE },
    { .pattern = "\\b(?:Task|ValueTask|Action|Func|Predicate|Comparison|EventHandler|IDisposable|IAsyncDisposable|IEquatable|IComparable|ICloneable)\\b", .token = HL_TOKEN_TYPE },
    { .pattern = "\\b(?:Nullable|Span|Memory|ReadOnlySpan|ReadOnlyMemory|ArraySegment|Range|Index)\\b", .token = HL_TOKEN_TYPE },
    { .pattern = "\\b(?:Console|Math|Convert|Environment|DateTime|TimeSpan|Guid|Uri|StringBuilder|Regex)\\b", .token = HL_TOKEN_TYPE },
    // Operators
    { .pattern = "[/*+:?&|%^~=!,<>.^-]+|\\?\\?|\\?\\.|\\.\\.", .token = HL_TOKEN_OPER },
    // PascalCase class names
    { .pattern = "\\b[A-Z][\\w_]*\\b", .token = HL_TOKEN_CLASS },
    // Method calls
    { .pattern = "[a-zA-Z_][\\w_]*(?=\\s*(?:<[^>]*>)?\\s*\\()", .token = HL_TOKEN_FUNC },
    // Boolean literals
    { .pattern = "\\b(?:true|false)\\b", .token = HL_TOKEN_BOOL },
    // Null literal
    { .pattern = "\\bnull\\b", .token = HL_TOKEN_NUM },
};

static const hl_detect_rule_t detect[] = {
    { "^using\\s+System", 500 },
    { "\\bnamespace\\s+\\w", 200 },
    { "\\bpublic\\s+(?:class|interface|struct|record)\\b", 100 },
    { "\\bConsole\\.(?:Write|Read)", 100 },
};

static const char* const aliases[] = { "cs", NULL };

static const hl_lang_def_t lang = {
    .name = "csharp",
    .aliases = aliases,
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = detect,
    .detect_count = sizeof(detect) / sizeof(detect[0]),
};

const hl_lang_def_t* hl_lang_csharp(void) { return &lang; }
