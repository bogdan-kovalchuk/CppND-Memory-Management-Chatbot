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
