#include <cassert>
#include <iostream>
#include <cstring>
#include <vector>
#include <memory>
#include <utility>

struct MockBitmap {
    char *data;
    MockBitmap() : data(nullptr) {}
    explicit MockBitmap(const char *s) {
        size_t n = std::strlen(s) + 1;
        data = new char[n];
        std::memcpy(data, s, n);
    }
    ~MockBitmap() { delete[] data; }
    MockBitmap(const MockBitmap &o) {
        if (o.data) {
            size_t n = std::strlen(o.data) + 1;
            data = new char[n];
            std::memcpy(data, o.data, n);
        }
    }
    MockBitmap &operator=(const MockBitmap &o) {
        if (this != &o) {
            delete[] data;
            if (o.data) {
                size_t n = std::strlen(o.data) + 1;
                data = new char[n];
                std::memcpy(data, o.data, n);
            } else {
                data = nullptr;
            }
        }
        return *this;
    }
    MockBitmap(MockBitmap &&o) noexcept : data(o.data) { o.data = nullptr; }
    MockBitmap &operator=(MockBitmap &&o) noexcept {
        if (this != &o) { delete[] data; data = o.data; o.data = nullptr; }
        return *this;
    }
};

struct MockChatBot {
    MockBitmap image;
    int *currentNode;

    MockChatBot() : currentNode(nullptr) {}
    explicit MockChatBot(const char *filename) : image(filename), currentNode(nullptr) {}

    MockChatBot(const MockChatBot &) = default;
    MockChatBot &operator=(const MockChatBot &) = default;
    MockChatBot(MockChatBot &&) noexcept = default;
    MockChatBot &operator=(MockChatBot &&) noexcept = default;
};

struct MockEdge {
    int id;
    MockEdge(int i) : id(i) {}
};

struct GraphNodeModel {
    int id;
    std::vector<std::unique_ptr<MockEdge>> childEdges;
    std::vector<MockEdge *> parentEdges;
    MockChatBot chatBot;

    explicit GraphNodeModel(int i) : id(i) {}

    GraphNodeModel(const GraphNodeModel &) = delete;
    GraphNodeModel &operator=(const GraphNodeModel &) = delete;
    GraphNodeModel(GraphNodeModel &&) noexcept = default;
    GraphNodeModel &operator=(GraphNodeModel &&) noexcept = default;

    void moveChatbotHere(MockChatBot bot) {
        chatBot = std::move(bot);
        chatBot.currentNode = &id;
    }

    void moveChatbotToNewNode(GraphNodeModel *newNode) {
        newNode->moveChatbotHere(std::move(chatBot));
    }

    void addChildEdge(std::unique_ptr<MockEdge> edge) {
        childEdges.push_back(std::move(edge));
    }

    void addParentEdge(MockEdge *edge) {
        parentEdges.push_back(edge);
    }
};

static void test_node_default_has_empty_chatbot() {
    GraphNodeModel node(1);
    assert(node.id == 1);
    assert(node.chatBot.image.data == nullptr);
    assert(node.chatBot.currentNode == nullptr);
    assert(node.childEdges.empty());
    assert(node.parentEdges.empty());
    std::cout << "  PASS: default node has empty chatbot and no edges\n";
}

static void test_move_chatbot_here() {
    GraphNodeModel node(1);
    MockChatBot bot("avatar.png");

    node.moveChatbotHere(std::move(bot));

    assert(node.chatBot.image.data != nullptr);
    assert(std::strcmp(node.chatBot.image.data, "avatar.png") == 0);
    assert(node.chatBot.currentNode == &node.id);
    assert(bot.image.data == nullptr);
    std::cout << "  PASS: moveChatbotHere transfers bot and sets current node\n";
}

static void test_move_chatbot_between_nodes() {
    GraphNodeModel src(1);
    GraphNodeModel dst(2);
    MockChatBot bot("move.png");

    src.moveChatbotHere(std::move(bot));
    assert(src.chatBot.image.data != nullptr);

    src.moveChatbotToNewNode(&dst);

    assert(dst.chatBot.image.data != nullptr);
    assert(std::strcmp(dst.chatBot.image.data, "move.png") == 0);
    assert(dst.chatBot.currentNode == &dst.id);
    assert(src.chatBot.image.data == nullptr);
    std::cout << "  PASS: moveChatbotToNewNode transfers bot between nodes\n";
}

static void test_node_move_constructor_transfers_edges() {
    GraphNodeModel original(1);
    original.addChildEdge(std::make_unique<MockEdge>(10));
    original.addChildEdge(std::make_unique<MockEdge>(20));

    MockEdge parentEdge(99);
    original.addParentEdge(&parentEdge);

    MockChatBot bot("node.png");
    original.moveChatbotHere(std::move(bot));

    GraphNodeModel moved(std::move(original));

    assert(moved.id == 1);
    assert(moved.childEdges.size() == 2);
    assert(moved.childEdges[0]->id == 10);
    assert(moved.childEdges[1]->id == 20);
    assert(moved.parentEdges.size() == 1);
    assert(moved.chatBot.image.data != nullptr);
    assert(std::strcmp(moved.chatBot.image.data, "node.png") == 0);

    assert(original.childEdges.empty());
    assert(original.chatBot.image.data == nullptr);
    std::cout << "  PASS: node move constructor transfers edges and chatbot\n";
}

static void test_node_move_assignment_replaces() {
    GraphNodeModel a(1);
    a.addChildEdge(std::make_unique<MockEdge>(10));
    a.moveChatbotHere(MockChatBot("first.png"));

    GraphNodeModel b(2);
    b.addChildEdge(std::make_unique<MockEdge>(20));
    b.moveChatbotHere(MockChatBot("second.png"));

    b = std::move(a);

    assert(b.id == 1);
    assert(b.childEdges.size() == 1);
    assert(b.childEdges[0]->id == 10);
    assert(std::strcmp(b.chatBot.image.data, "first.png") == 0);
    assert(a.childEdges.empty());
    assert(a.chatBot.image.data == nullptr);
    std::cout << "  PASS: node move-assignment replaces content\n";
}

static void test_chatbot_current_node_pointer_updates() {
    GraphNodeModel nodeA(1);
    GraphNodeModel nodeB(2);
    MockChatBot bot("ptr.png");

    nodeA.moveChatbotHere(std::move(bot));
    assert(nodeA.chatBot.currentNode == &nodeA.id);

    nodeA.moveChatbotToNewNode(&nodeB);
    assert(nodeB.chatBot.currentNode == &nodeB.id);
    std::cout << "  PASS: chatbot currentNode pointer updates on transfer\n";
}

static void test_transfer_chain_clears_intermediate_nodes() {
    GraphNodeModel first(1);
    GraphNodeModel middle(2);
    GraphNodeModel last(3);
    first.moveChatbotHere(MockChatBot("chain.png"));

    first.moveChatbotToNewNode(&middle);
    middle.moveChatbotToNewNode(&last);

    assert(first.chatBot.image.data == nullptr);
    assert(middle.chatBot.image.data == nullptr);
    assert(last.chatBot.image.data != nullptr);
    assert(std::strcmp(last.chatBot.image.data, "chain.png") == 0);
    assert(last.chatBot.currentNode == &last.id);
    std::cout << "  PASS: transfer chain clears intermediate nodes\n";
}

int main() {
    std::cout << "GraphNode move semantics characterization:\n";
    test_node_default_has_empty_chatbot();
    test_move_chatbot_here();
    test_move_chatbot_between_nodes();
    test_node_move_constructor_transfers_edges();
    test_node_move_assignment_replaces();
    test_chatbot_current_node_pointer_updates();
    test_transfer_chain_clears_intermediate_nodes();
    std::cout << "All graph-node characterization tests passed.\n";
    return 0;
}
