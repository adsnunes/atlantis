// atlantis_chat.h

#ifndef ATLANTIS_CHAT_H
#define ATLANTIS_CHAT_H

#include "atlantis_types.h"

// #region Message Management

//! Add a message to the chat history
//! @param text message content
//! @param is_user true for user messages, false for AI responses
void chat_add(const char* text, bool is_user);

//! Clear all chat messages and free memory
void chat_clear(void);

// #endregion

#if HAS_LIBAI

// #region AI Session

//! Initialize AI session with system prompt and tools
//! Creates session with document, search, time, and history tools
void ai_init_session(void);

//! Send a prompt to the AI and stream the response
//! @param prompt user's input message
void ai_send(const char* prompt);

// #endregion

#endif // HAS_LIBAI

#endif // ATLANTIS_CHAT_H
