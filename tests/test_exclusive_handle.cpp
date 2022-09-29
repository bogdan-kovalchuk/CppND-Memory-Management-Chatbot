#include <cassert>
#include <iostream>
#include <cstring>
#include "exclusive_handle.h"

struct Payload {
    char *data;
    size_t size;

    Payload() : data(nullptr), size(0) {}
    explicit Payload(const char *src) {
        size = std::strlen(src) + 1;
        data = new char[size];
        std::memcpy(data, src, size);
    }
    ~Payload() { delete[] data; }

    static Payload *clone(const Payload *p) {
        if (!p) return nullptr;
        return new Payload(p->data ? p->data : "");
    }
};

static void test_default_is_null() {
    ExclusiveHandle<Payload> h;
    assert(!h);
    assert(h.get() == nullptr);
    std::cout << "  PASS: default handle is null\n";
}

static void test_acquire_owns_resource() {
    ExclusiveHandle<Payload> h(new Payload("hello"), Payload::clone);
    assert(h);
    assert(std::strcmp(h->data, "hello") == 0);
    std::cout << "  PASS: handle acquires and owns resource\n";
}

static void test_copy_deep_clones() {
    ExclusiveHandle<Payload> a(new Payload("original"), Payload::clone);
    ExclusiveHandle<Payload> b(a);

    assert(a.get() != b.get());
    assert(std::strcmp(b->data, "original") == 0);
    std::cout << "  PASS: copy deep-clones via clone function\n";
}

static void test_copy_assign_replaces() {
    ExclusiveHandle<Payload> a(new Payload("src"), Payload::clone);
    ExclusiveHandle<Payload> b(new Payload("dst"), Payload::clone);

    b = a;
    assert(a.get() != b.get());
    assert(std::strcmp(b->data, "src") == 0);
    assert(std::strcmp(a->data, "src") == 0);
    std::cout << "  PASS: copy-assign replaces destination\n";
}

static void test_self_copy_assign_is_safe() {
    ExclusiveHandle<Payload> a(new Payload("self"), Payload::clone);
    a = a;
    assert(a);
    assert(std::strcmp(a->data, "self") == 0);
    std::cout << "  PASS: self copy-assign is no-op\n";
}

static void test_move_transfers() {
    ExclusiveHandle<Payload> a(new Payload("mover"), Payload::clone);
    Payload *raw = a.get();

    ExclusiveHandle<Payload> b(std::move(a));
    assert(b.get() == raw);
    assert(!a);
    std::cout << "  PASS: move transfers ownership\n";
}

static void test_move_assign_transfers_and_releases() {
    ExclusiveHandle<Payload> a(new Payload("new"), Payload::clone);
    ExclusiveHandle<Payload> b(new Payload("old"), Payload::clone);

    b = std::move(a);
    assert(!a);
    assert(b);
    assert(std::strcmp(b->data, "new") == 0);
    std::cout << "  PASS: move-assign transfers and releases old\n";
}

static void test_self_move_assign_is_safe() {
    ExclusiveHandle<Payload> a(new Payload("safe"), Payload::clone);
    ExclusiveHandle<Payload> &ref = a;
    a = std::move(ref);
    assert(a);
    assert(std::strcmp(a->data, "safe") == 0);
    std::cout << "  PASS: self move-assign preserves state\n";
}

static void test_release_yields_ownership() {
    ExclusiveHandle<Payload> a(new Payload("release"), Payload::clone);
    Payload *raw = a.release();
    assert(!a);
    assert(raw != nullptr);
    assert(std::strcmp(raw->data, "release") == 0);
    delete raw;
    std::cout << "  PASS: release yields ownership\n";
}

static void test_reset_replaces_resource() {
    ExclusiveHandle<Payload> a(new Payload("first"), Payload::clone);
    a.reset(new Payload("second"));
    assert(std::strcmp(a->data, "second") == 0);
    std::cout << "  PASS: reset replaces resource\n";
}

static void test_release_then_reset_owns_replacement() {
    ExclusiveHandle<Payload> handle(new Payload("released"), Payload::clone);
    Payload *released = handle.release();

    assert(!handle);
    assert(released != nullptr);
    delete released;

    handle.reset(new Payload("replacement"));
    assert(handle);
    assert(std::strcmp(handle->data, "replacement") == 0);
    std::cout << "  PASS: release leaves no ownership before reset replacement\n";
}

static void test_swap_exchanges() {
    ExclusiveHandle<Payload> a(new Payload("A"), Payload::clone);
    ExclusiveHandle<Payload> b(new Payload("B"), Payload::clone);

    using std::swap;
    swap(a, b);
    assert(std::strcmp(a->data, "B") == 0);
    assert(std::strcmp(b->data, "A") == 0);
    std::cout << "  PASS: swap exchanges handles\n";
}

static void test_copy_without_clone_is_null() {
    ExclusiveHandle<Payload> a(new Payload("data"));
    ExclusiveHandle<Payload> b(a);
    assert(!b);
    std::cout << "  PASS: copy without clone function yields null\n";
}

int main() {
    std::cout << "ExclusiveHandle tests:\n";
    test_default_is_null();
    test_acquire_owns_resource();
    test_copy_deep_clones();
    test_copy_assign_replaces();
    test_self_copy_assign_is_safe();
    test_move_transfers();
    test_move_assign_transfers_and_releases();
    test_self_move_assign_is_safe();
    test_release_yields_ownership();
    test_reset_replaces_resource();
    test_release_then_reset_owns_replacement();
    test_swap_exchanges();
    test_copy_without_clone_is_null();
    std::cout << "All ExclusiveHandle tests passed.\n";
    return 0;
}
