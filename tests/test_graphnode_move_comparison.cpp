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

struct NodeMoveAssign {
    int id;
    std::vector<std::unique_ptr<int>> childEdges;
    MockChatBot chatBot;

    explicit NodeMoveAssign(int i) : id(i) {}
    NodeMoveAssign(const NodeMoveAssign &) = delete;
    NodeMoveAssign &operator=(const NodeMoveAssign &) = delete;
    NodeMoveAssign(NodeMoveAssign &&) noexcept = default;
    NodeMoveAssign &operator=(NodeMoveAssign &&) noexcept = default;

    void moveChatbotHere(MockChatBot bot) {
        chatBot = std::move(bot);
        chatBot.currentNode = &id;
    }
    void moveChatbotToNewNode(NodeMoveAssign *newNode) {
        newNode->moveChatbotHere(std::move(chatBot));
    }
};

struct NodeSwap {
    int id;
    std::vector<std::unique_ptr<int>> childEdges;
    MockChatBot chatBot;

    explicit NodeSwap(int i) : id(i) {}
    NodeSwap(const NodeSwap &) = delete;
    NodeSwap &operator=(const NodeSwap &) = delete;
    NodeSwap(NodeSwap &&) noexcept = default;
    NodeSwap &operator=(NodeSwap &&) noexcept = default;

    void moveChatbotHere(MockChatBot bot) {
        std::swap(chatBot, bot);
        chatBot.currentNode = &id;
    }
    void moveChatbotToNewNode(NodeSwap *newNode) {
        newNode->moveChatbotHere(std::move(chatBot));
    }
};

static void test_both_approaches_transfer_image() {
    NodeMoveAssign ma_src(1), ma_dst(2);
    ma_src.moveChatbotHere(MockChatBot("test.png"));
    ma_src.moveChatbotToNewNode(&ma_dst);

    NodeSwap sw_src(1), sw_dst(2);
    sw_src.moveChatbotHere(MockChatBot("test.png"));
    sw_src.moveChatbotToNewNode(&sw_dst);

    assert(std::strcmp(ma_dst.chatBot.image.data, "test.png") == 0);
    assert(std::strcmp(sw_dst.chatBot.image.data, "test.png") == 0);
    assert(ma_src.chatBot.image.data == nullptr);
    assert(sw_src.chatBot.image.data == nullptr);
    std::cout << "  PASS: both approaches transfer image identically\n";
}

static void test_both_approaches_set_current_node() {
    NodeMoveAssign ma_src(1), ma_dst(2);
    ma_src.moveChatbotHere(MockChatBot("a.png"));
    ma_src.moveChatbotToNewNode(&ma_dst);

    NodeSwap sw_src(1), sw_dst(2);
    sw_src.moveChatbotHere(MockChatBot("a.png"));
    sw_src.moveChatbotToNewNode(&sw_dst);

    assert(ma_dst.chatBot.currentNode == &ma_dst.id);
    assert(sw_dst.chatBot.currentNode == &sw_dst.id);
    std::cout << "  PASS: both approaches set currentNode to destination\n";
}

static void test_both_approaches_chain_transfer() {
    NodeMoveAssign ma[4] = {NodeMoveAssign(0), NodeMoveAssign(1), NodeMoveAssign(2), NodeMoveAssign(3)};
    ma[0].moveChatbotHere(MockChatBot("chain.png"));
    ma[0].moveChatbotToNewNode(&ma[1]);
    ma[1].moveChatbotToNewNode(&ma[2]);
    ma[2].moveChatbotToNewNode(&ma[3]);

    NodeSwap sw[4] = {NodeSwap(0), NodeSwap(1), NodeSwap(2), NodeSwap(3)};
    sw[0].moveChatbotHere(MockChatBot("chain.png"));
    sw[0].moveChatbotToNewNode(&sw[1]);
    sw[1].moveChatbotToNewNode(&sw[2]);
    sw[2].moveChatbotToNewNode(&sw[3]);

    assert(std::strcmp(ma[3].chatBot.image.data, "chain.png") == 0);
    assert(std::strcmp(sw[3].chatBot.image.data, "chain.png") == 0);
    for (int i = 0; i < 3; ++i) {
        assert(ma[i].chatBot.image.data == nullptr);
        assert(sw[i].chatBot.image.data == nullptr);
    }
    std::cout << "  PASS: both approaches produce identical chain transfers\n";
}

static void test_swap_self_transfer_differs() {
    NodeSwap node(1);
    node.moveChatbotHere(MockChatBot("self.png"));
    node.moveChatbotToNewNode(&node);

    assert(node.chatBot.image.data != nullptr);
    assert(std::strcmp(node.chatBot.image.data, "self.png") == 0);
    std::cout << "  PASS: swap self-transfer preserves image (round-trip via param)\n";
}

static void test_both_approaches_empty_bot_transfer() {
    NodeMoveAssign ma_src(1), ma_dst(2);
    ma_src.moveChatbotToNewNode(&ma_dst);

    NodeSwap sw_src(1), sw_dst(2);
    sw_src.moveChatbotToNewNode(&sw_dst);

    assert(ma_dst.chatBot.image.data == nullptr);
    assert(sw_dst.chatBot.image.data == nullptr);
    assert(ma_dst.chatBot.currentNode == &ma_dst.id);
    assert(sw_dst.chatBot.currentNode == &sw_dst.id);
    std::cout << "  PASS: both approaches handle empty bot transfer identically\n";
}

int main() {
    std::cout << "GraphNode move approach comparison:\n";
    test_both_approaches_transfer_image();
    test_both_approaches_set_current_node();
    test_both_approaches_chain_transfer();
    test_swap_self_transfer_differs();
    test_both_approaches_empty_bot_transfer();
    std::cout << "All comparison tests passed.\n";
    return 0;
}
