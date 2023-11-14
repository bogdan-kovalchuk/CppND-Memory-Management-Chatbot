# ChatBot Ownership Model

## Overview

ChatBot manages one owned resource (`wxBitmap` avatar via `ExclusiveHandle`)
and three non-owning back-pointers (`GraphNode *currentNode`,
`GraphNode *rootNode`, `ChatLogic *chatLogic`).

## Owned resource: `_image`

| Operation | Complexity | Behaviour |
|-----------|-----------|-----------|
| Default construct | O(1) | Null handle, no allocation |
| Construct from file | O(W*H) | Allocates and loads bitmap |
| Copy construct | O(W*H) | Deep clone via `cloneWxBitmap` |
| Copy assign | O(W*H) | Copy-and-swap; self-assign is no-op |
| Move construct | O(1) | Pointer transfer, source nulled |
| Move assign | O(1) | Releases old, transfers, source nulled |

`ExclusiveHandle<T>` stores a type-erased clone function so that deep-copy
semantics are supplied by the caller rather than baked into the handle. This
differs from `unique_ptr` (non-copyable) and `shared_ptr` (reference-counted).

## Non-owning pointers

The back-pointers are set by `ChatLogic` after construction and must outlive
the ChatBot. Copy operations replicate the pointers; move operations transfer
them and null the source. All dereference sites guard against null.

## Cycle considerations

`ChatLogic` owns `GraphNode`s (via `unique_ptr`); each `GraphNode` contains a
`ChatBot` by value; `ChatBot` holds a non-owning pointer back to `ChatLogic`.
The non-owning back-pointer breaks what would otherwise be a reference cycle.

## Thread safety

None. ChatBot is designed for single-threaded GUI event-loop use.

## Graph ownership

`ChatLogic` exclusively owns every `GraphNode` via `std::vector<std::unique_ptr<GraphNode>>`.
Each `GraphNode` exclusively owns its outgoing `GraphEdge`s via
`std::vector<std::unique_ptr<GraphEdge>>` and holds non-owning raw pointers to
its incoming edges (`_parentEdges`). Each `GraphEdge` holds non-owning raw
pointers to its parent and child `GraphNode` (initialized to `nullptr` in the
constructor, then set once via `SetParentNode`/`SetChildNode` while the graph
is being built from the answer-graph file).

`LoadAnswerGraphFromFile` guards against malformed input before wiring these
pointers: an `EDGE` line referencing an unknown `PARENT`/`CHILD` id is skipped
with a diagnostic instead of dereferencing `_nodes.end()`, and the root-node
search bails out with a diagnostic instead of dereferencing a null `rootNode`
when the file defines no unambiguous root.

## Chatbot transfer protocol

Each `GraphNode` holds its `ChatBot` by value. Moving the active chatbot
between nodes goes through `GraphNode::MoveChatbotHere(ChatBot chatbot)`,
which takes the bot by value and swaps it into `_chatBot`:

```cpp
void GraphNode::MoveChatbotHere(ChatBot chatbot)
{
    std::swap(_chatBot, chatbot);
    _chatBot.SetCurrentNode(this);
}
```

Callers must pass an rvalue so the by-value parameter binds to `ChatBot`'s
move constructor (O(1), pointer transfer) rather than its copy constructor
(O(W*H), deep-clones the avatar image). `ChatLogic::LoadAnswerGraphFromFile`
does this explicitly with `std::move` when attaching the initial bot to the
root node, and `GraphNode::MoveChatbotToNewNode` does the same when handing
the bot off to the next node in the conversation:

```cpp
void GraphNode::MoveChatbotToNewNode(GraphNode *newNode)
{
    if (newNode)
        newNode->MoveChatbotHere(std::move(_chatBot));
}
```

Because the parameter is constructed by moving from `_chatBot` before the
swap runs, self-transfer (`node.MoveChatbotToNewNode(&node)`) round-trips
safely: the swap puts the (already-moved-from) member and the local parameter
back the way they started.
