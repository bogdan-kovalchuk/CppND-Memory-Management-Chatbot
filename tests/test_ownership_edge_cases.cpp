#include <cassert>
#include <iostream>
#include <cstring>
#include <stdexcept>
#include "levenshtein.h"

struct ImageResource {
    char *data;
    size_t size;

    ImageResource() : data(nullptr), size(0) {}
    ImageResource(const char *src) {
        size = std::strlen(src) + 1;
        data = new char[size];
        std::memcpy(data, src, size);
    }
    ~ImageResource() { delete[] data; }

    ImageResource(const ImageResource &o) {
        size = o.size;
        data = new char[size];
        std::memcpy(data, o.data, size);
    }
    ImageResource &operator=(const ImageResource &o) {
        if (this != &o) {
            delete[] data;
            size = o.size;
            data = new char[size];
            std::memcpy(data, o.data, size);
        }
        return *this;
    }
    ImageResource(ImageResource &&o) noexcept : data(o.data), size(o.size) {
        o.data = nullptr;
        o.size = 0;
    }
    ImageResource &operator=(ImageResource &&o) noexcept {
        if (this != &o) {
            delete[] data;
            data = o.data;
            size = o.size;
            o.data = nullptr;
            o.size = 0;
        }
        return *this;
    }
};

struct ChatBotModel {
    ImageResource *image;
    int *currentNode;
    int *rootNode;

    ChatBotModel() : image(nullptr), currentNode(nullptr), rootNode(nullptr) {}
    explicit ChatBotModel(const char *filename) : image(new ImageResource(filename)),
                                                   currentNode(nullptr), rootNode(nullptr) {}
    ~ChatBotModel() { delete image; }

    ChatBotModel(const ChatBotModel &o) {
        image = new ImageResource(*o.image);
        currentNode = o.currentNode;
        rootNode = o.rootNode;
    }
    ChatBotModel &operator=(const ChatBotModel &o) {
        if (this == &o) return *this;
        delete image;
        image = new ImageResource(*o.image);
        currentNode = o.currentNode;
        rootNode = o.rootNode;
        return *this;
    }
    ChatBotModel(ChatBotModel &&o) noexcept {
        image = o.image;
        currentNode = o.currentNode;
        rootNode = o.rootNode;
        o.image = nullptr;
        o.currentNode = nullptr;
        o.rootNode = nullptr;
    }
    ChatBotModel &operator=(ChatBotModel &&o) noexcept {
        if (this == &o) return *this;
        delete image;
        image = o.image;
        currentNode = o.currentNode;
        rootNode = o.rootNode;
        o.image = nullptr;
        o.currentNode = nullptr;
        o.rootNode = nullptr;
        return *this;
    }
};

static void test_copy_from_null_image_is_unsafe() {
    ChatBotModel empty;
    assert(empty.image == nullptr);
    assert(empty.currentNode == nullptr);
    assert(empty.rootNode == nullptr);
    std::cout << "  PASS: default-constructed bot has all-null handles\n";
}

static void test_self_assignment_is_safe() {
    ChatBotModel bot("self.png");
    bot = bot;
    assert(bot.image != nullptr);
    assert(std::strcmp(bot.image->data, "self.png") == 0);
    std::cout << "  PASS: self copy-assignment is guarded\n";
}

static void test_self_move_assignment_is_safe() {
    ChatBotModel bot("self.png");
    ChatBotModel &ref = bot;
    bot = std::move(ref);
    assert(bot.image != nullptr);
    assert(std::strcmp(bot.image->data, "self.png") == 0);
    std::cout << "  PASS: self move-assignment is guarded and preserves state\n";
}

static void test_double_move_leaves_source_empty() {
    ChatBotModel src("data.png");
    ChatBotModel dst1(std::move(src));
    assert(src.image == nullptr);

    ChatBotModel dst2(std::move(src));
    assert(dst2.image == nullptr);
    std::cout << "  PASS: moving from already-moved source yields null\n";
}

static void test_levenshtein_single_char() {
    assert(computeLevenshteinDistance("a", "b") == 1);
    assert(computeLevenshteinDistance("a", "a") == 0);
    assert(computeLevenshteinDistance("", "") == 0);
    std::cout << "  PASS: levenshtein single-char edge cases\n";
}

static void test_move_assign_releases_old_image() {
    ChatBotModel a("new.png");
    ChatBotModel b("old.png");

    ImageResource *old_ptr = b.image;
    b = std::move(a);

    assert(b.image != old_ptr);
    assert(a.image == nullptr);
    assert(std::strcmp(b.image->data, "new.png") == 0);
    std::cout << "  PASS: move-assign releases old destination image\n";
}

int main() {
    std::cout << "ChatBot ownership edge cases:\n";
    test_copy_from_null_image_is_unsafe();
    test_self_assignment_is_safe();
    test_self_move_assignment_is_safe();
    test_double_move_leaves_source_empty();
    test_levenshtein_single_char();
    test_move_assign_releases_old_image();
    std::cout << "All edge case tests passed.\n";
    return 0;
}
