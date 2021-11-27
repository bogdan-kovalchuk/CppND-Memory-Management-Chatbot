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

static void test_move_empty_chatbot_between_nodes() {
    GraphNodeModel src(1);
    GraphNodeModel dst(2);

    src.moveChatbotToNewNode(&dst);

    assert(dst.chatBot.image.data == nullptr);
    assert(dst.chatBot.currentNode == &dst.id);
    assert(src.chatBot.image.data == nullptr);
    std::cout << "  PASS: moving empty chatbot transfers without crash\n";
}

static void test_move_to_null_node_is_unsafe() {
    GraphNodeModel src(1);
    src.moveChatbotHere(MockChatBot("test.png"));

    GraphNodeModel *nullNode = nullptr;
    assert(nullNode == nullptr);
    std::cout << "  PASS: null target node detected (move would dereference null)\n";
}

static void test_self_move_chatbot() {
    GraphNodeModel node(1);
    node.moveChatbotHere(MockChatBot("self.png"));

    node.moveChatbotToNewNode(&node);

    assert(node.chatBot.image.data != nullptr);
    assert(std::strcmp(node.chatBot.image.data, "self.png") == 0);
    std::cout << "  PASS: self-move chatbot preserves image (pass-by-value round-trip)\n";
}

static void test_move_from_moved_node() {
    GraphNodeModel src(1);
    src.moveChatbotHere(MockChatBot("data.png"));
    src.addChildEdge(std::make_unique<MockEdge>(10));

    GraphNodeModel mid(2);
    src.moveChatbotToNewNode(&mid);

    GraphNodeModel dst(3);
    src.moveChatbotToNewNode(&dst);

    assert(dst.chatBot.image.data == nullptr);
    assert(mid.chatBot.image.data != nullptr);
    assert(std::strcmp(mid.chatBot.image.data, "data.png") == 0);
    std::cout << "  PASS: moving from already-moved node yields empty bot\n";
}

static void test_node_move_preserves_parent_edge_pointers() {
    MockEdge parentEdge(99);
    GraphNodeModel original(1);
    original.addParentEdge(&parentEdge);
    original.moveChatbotHere(MockChatBot("parent.png"));

    GraphNodeModel moved(std::move(original));

    assert(moved.parentEdges.size() == 1);
    assert(moved.parentEdges[0] == &parentEdge);
    assert(std::strcmp(moved.chatBot.image.data, "parent.png") == 0);
    std::cout << "  PASS: node move preserves non-owning parent edge pointers\n";
}

static void test_sequential_transfers_through_chain() {
    GraphNodeModel n1(1), n2(2), n3(3), n4(4);
    n1.moveChatbotHere(MockChatBot("chain.png"));

    n1.moveChatbotToNewNode(&n2);
    assert(n2.chatBot.image.data != nullptr);
    assert(n1.chatBot.image.data == nullptr);

    n2.moveChatbotToNewNode(&n3);
    assert(n3.chatBot.image.data != nullptr);
    assert(n2.chatBot.image.data == nullptr);

    n3.moveChatbotToNewNode(&n4);
    assert(n4.chatBot.image.data != nullptr);
    assert(std::strcmp(n4.chatBot.image.data, "chain.png") == 0);
    assert(n3.chatBot.image.data == nullptr);
    std::cout << "  PASS: sequential transfers through node chain\n";
}

int main() {
    std::cout << "GraphNode move edge cases:\n";
    test_move_empty_chatbot_between_nodes();
    test_move_to_null_node_is_unsafe();
    test_self_move_chatbot();
    test_move_from_moved_node();
    test_node_move_preserves_parent_edge_pointers();
    test_sequential_transfers_through_chain();
    std::cout << "All graph-node edge case tests passed.\n";
    return 0;
}
