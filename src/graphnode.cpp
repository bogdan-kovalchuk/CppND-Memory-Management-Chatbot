#include "graphedge.h"
#include "graphnode.h"
#include <utility>

GraphNode::GraphNode(int id)
{
    _id = id;
}

GraphNode::~GraphNode()
{
    // _childEdges are owned via unique_ptr and _chatBot is a value member;
    // both clean up automatically, no manual deletion needed.
}

void GraphNode::AddToken(std::string token)
{
    _answers.push_back(token);
}

void GraphNode::AddEdgeToParentNode(GraphEdge *edge)
{
    _parentEdges.push_back(edge);
}

void GraphNode::AddEdgeToChildNode(std::unique_ptr<GraphEdge> edge)
{
    _childEdges.push_back(std::move(edge));
}

//// STUDENT CODE
////
void GraphNode::MoveChatbotHere(ChatBot chatbot)
{
    std::swap(_chatBot, chatbot);
    _chatBot.SetCurrentNode(this);
}

void GraphNode::MoveChatbotToNewNode(GraphNode *newNode)
{
    // without std::move, the by-value parameter of MoveChatbotHere binds to
    // the copy constructor, deep-cloning the avatar image on every transfer
    // instead of moving it as documented in docs/ownership.md.
    if (newNode)
        newNode->MoveChatbotHere(std::move(_chatBot));
}
////
//// EOF STUDENT CODE

GraphEdge *GraphNode::GetChildEdgeAtIndex(size_t index)
{
    //// STUDENT CODE
    ////

    if (index >= _childEdges.size())
        return nullptr;
    return _childEdges[index].get();

    ////
    //// EOF STUDENT CODE
}