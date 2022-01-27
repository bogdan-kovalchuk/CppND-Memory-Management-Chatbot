#include <cassert>
#include <iostream>
#include <cstring>
#include <string>
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

static void test_levenshtein_basic() {
    assert(computeLevenshteinDistance("kitten", "sitting") == 3);
    assert(computeLevenshteinDistance("", "abc") == 3);
    assert(computeLevenshteinDistance("abc", "") == 3);
    assert(computeLevenshteinDistance("abc", "abc") == 0);
    assert(computeLevenshteinDistance("ABC", "abc") == 0);
    std::cout << "  PASS: levenshtein basic\n";
}

static void test_raw_pointer_ownership_copy() {
    int nodeA = 1, nodeB = 2;
    ChatBotModel original("chatbot.png");
    original.currentNode = &nodeA;
    original.rootNode = &nodeB;

    ChatBotModel copied(original);

    assert(copied.image != nullptr);
    assert(copied.image != original.image);
    assert(std::strcmp(copied.image->data, "chatbot.png") == 0);
    assert(copied.currentNode == &nodeA);
    assert(copied.rootNode == &nodeB);
    std::cout << "  PASS: raw-pointer copy creates deep image clone\n";
}

static void test_raw_pointer_ownership_move() {
    int nodeA = 10;
    ChatBotModel source("avatar.png");
    source.currentNode = &nodeA;

    ChatBotModel dest(std::move(source));

    assert(dest.image != nullptr);
    assert(std::strcmp(dest.image->data, "avatar.png") == 0);
    assert(source.image == nullptr);
    assert(dest.currentNode == &nodeA);
    assert(source.currentNode == nullptr);
    std::cout << "  PASS: raw-pointer move transfers image and nulls source\n";
}

static void test_copy_assignment_replaces_image() {
    ChatBotModel a("first.png");
    ChatBotModel b("second.png");

    b = a;

    assert(b.image != nullptr);
    assert(b.image != a.image);
    assert(std::strcmp(b.image->data, "first.png") == 0);
    assert(std::strcmp(a.image->data, "first.png") == 0);
    std::cout << "  PASS: copy assignment replaces destination image\n";
}

static void test_move_assignment_transfers() {
    ChatBotModel a("src.png");
    ChatBotModel b("dst.png");

    b = std::move(a);

    assert(b.image != nullptr);
    assert(std::strcmp(b.image->data, "src.png") == 0);
    assert(a.image == nullptr);
    std::cout << "  PASS: move assignment transfers and nulls source\n";
}

static void test_copy_remains_independent_after_original_moves() {
    ChatBotModel original("independent.png");
    ChatBotModel copied(original);

    ChatBotModel moved(std::move(original));

    assert(copied.image != nullptr);
    assert(copied.image != moved.image);
    assert(std::strcmp(copied.image->data, "independent.png") == 0);
    assert(moved.image != nullptr);
    assert(original.image == nullptr);
    std::cout << "  PASS: copied image remains independent after original moves\n";
}

int main() {
    std::cout << "ChatBot ownership characterization:\n";
    test_levenshtein_basic();
    test_raw_pointer_ownership_copy();
    test_raw_pointer_ownership_move();
    test_copy_assignment_replaces_image();
    test_move_assignment_transfers();
    test_copy_remains_independent_after_original_moves();
    std::cout << "All characterization tests passed.\n";
    return 0;
}
