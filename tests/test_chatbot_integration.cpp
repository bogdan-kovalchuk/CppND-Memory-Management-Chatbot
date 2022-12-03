#include <cassert>
#include <iostream>
#include <cstring>
#include "exclusive_handle.h"

struct MockBitmap {
    char *pixels;
    int width, height;

    MockBitmap() : pixels(nullptr), width(0), height(0) {}
    MockBitmap(const char *filename, int w, int h) : width(w), height(h) {
        pixels = new char[w * h];
        std::memset(pixels, 0, w * h);
        std::strncpy(pixels, filename, w * h - 1);
        pixels[w * h - 1] = '\0';
    }
    ~MockBitmap() { delete[] pixels; }

    static MockBitmap *clone(const MockBitmap *src) {
        if (!src || !src->pixels) return nullptr;
        auto *copy = new MockBitmap();
        copy->width = src->width;
        copy->height = src->height;
        copy->pixels = new char[src->width * src->height];
        std::memcpy(copy->pixels, src->pixels, src->width * src->height);
        return copy;
    }
};

struct ChatBotLike {
    ExclusiveHandle<MockBitmap> image;
    int *currentNode;
    int *rootNode;

    ChatBotLike() : currentNode(nullptr), rootNode(nullptr) {}
    explicit ChatBotLike(const char *filename)
        : image(new MockBitmap(filename, 32, 32), MockBitmap::clone),
          currentNode(nullptr), rootNode(nullptr) {}

    ChatBotLike(const ChatBotLike &o)
        : image(o.image), currentNode(o.currentNode), rootNode(o.rootNode) {}

    ChatBotLike &operator=(const ChatBotLike &o) {
        if (this != &o) {
            image = o.image;
            currentNode = o.currentNode;
            rootNode = o.rootNode;
        }
        return *this;
    }

    ChatBotLike(ChatBotLike &&o) noexcept
        : image(std::move(o.image)), currentNode(o.currentNode), rootNode(o.rootNode) {
        o.currentNode = nullptr;
        o.rootNode = nullptr;
    }

    ChatBotLike &operator=(ChatBotLike &&o) noexcept {
        if (this != &o) {
            image = std::move(o.image);
            currentNode = o.currentNode;
            rootNode = o.rootNode;
            o.currentNode = nullptr;
            o.rootNode = nullptr;
        }
        return *this;
    }
};

static void test_chatbot_default_has_null_image() {
    ChatBotLike bot;
    assert(!bot.image);
    assert(bot.currentNode == nullptr);
    std::cout << "  PASS: default ChatBot has null image\n";
}

static void test_chatbot_construct_with_image() {
    ChatBotLike bot("avatar.png");
    assert(bot.image);
    assert(bot.image->width == 32);
    assert(std::strcmp(bot.image->pixels, "avatar.png") == 0);
    std::cout << "  PASS: ChatBot constructs with owned image\n";
}

static void test_chatbot_copy_deep_clones_image() {
    int nodeA = 1;
    ChatBotLike original("chat.png");
    original.currentNode = &nodeA;

    ChatBotLike copied(original);
    assert(copied.image);
    assert(copied.image.get() != original.image.get());
    assert(std::strcmp(copied.image->pixels, "chat.png") == 0);
    assert(copied.currentNode == &nodeA);
    std::cout << "  PASS: ChatBot copy deep-clones image via ExclusiveHandle\n";
}

static void test_chatbot_move_transfers_image() {
    int nodeA = 42;
    ChatBotLike src("move.png");
    src.currentNode = &nodeA;

    ChatBotLike dst(std::move(src));
    assert(dst.image);
    assert(!src.image);
    assert(std::strcmp(dst.image->pixels, "move.png") == 0);
    assert(dst.currentNode == &nodeA);
    assert(src.currentNode == nullptr);
    std::cout << "  PASS: ChatBot move transfers image and nulls source\n";
}

static void test_chatbot_copy_assign_replaces_image() {
    ChatBotLike a("first.png");
    ChatBotLike b("second.png");

    b = a;
    assert(b.image);
    assert(b.image.get() != a.image.get());
    assert(std::strcmp(b.image->pixels, "first.png") == 0);
    std::cout << "  PASS: ChatBot copy-assign replaces image\n";
}

static void test_chatbot_move_assign_releases_old() {
    ChatBotLike a("new.png");
    ChatBotLike b("old.png");

    MockBitmap *old_raw = b.image.get();
    b = std::move(a);

    assert(b.image);
    assert(!a.image);
    assert(b.image.get() != old_raw);
    assert(std::strcmp(b.image->pixels, "new.png") == 0);
    std::cout << "  PASS: ChatBot move-assign releases old image\n";
}

static void test_chatbot_self_copy_assign_preserves() {
    ChatBotLike bot("self.png");
    bot = bot;
    assert(bot.image);
    assert(std::strcmp(bot.image->pixels, "self.png") == 0);
    std::cout << "  PASS: ChatBot self copy-assign preserves state\n";
}

static void test_chatbot_self_move_assign_preserves() {
    ChatBotLike bot("self.png");
    ChatBotLike &ref = bot;
    bot = std::move(ref);
    assert(bot.image);
    assert(std::strcmp(bot.image->pixels, "self.png") == 0);
    std::cout << "  PASS: ChatBot self move-assign preserves state\n";
}

static void test_copy_move_sequence_preserves_image_and_node_handles() {
    int current = 7;
    int root = 9;
    ChatBotLike original("sequence.png");
    original.currentNode = &current;
    original.rootNode = &root;

    ChatBotLike copied(original);
    ChatBotLike moved(std::move(original));

    assert(copied.image.get() != moved.image.get());
    assert(std::strcmp(copied.image->pixels, "sequence.png") == 0);
    assert(std::strcmp(moved.image->pixels, "sequence.png") == 0);
    assert(copied.currentNode == &current);
    assert(copied.rootNode == &root);
    assert(moved.currentNode == &current);
    assert(moved.rootNode == &root);
    assert(original.currentNode == nullptr);
    assert(original.rootNode == nullptr);
    std::cout << "  PASS: copy and move preserve intended ownership and node handles\n";
}

int main() {
    std::cout << "ChatBot integration with ExclusiveHandle:\n";
    test_chatbot_default_has_null_image();
    test_chatbot_construct_with_image();
    test_chatbot_copy_deep_clones_image();
    test_chatbot_move_transfers_image();
    test_chatbot_copy_assign_replaces_image();
    test_chatbot_move_assign_releases_old();
    test_chatbot_self_copy_assign_preserves();
    test_chatbot_self_move_assign_preserves();
    test_copy_move_sequence_preserves_image_and_node_handles();
    std::cout << "All integration tests passed.\n";
    return 0;
}
