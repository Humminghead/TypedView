

## Introdution

When I see NativePointer realization of part of code of [Frida’s API](https://frida.re/docs/javascript-api/), I thought: "It's cool. I want to see it in my favorite programming language."
The main goal of this design is to work with data without copying it, and represent access to the same memory location in different types.

## Requirements

Compiler what suport C++17

## Examples

```cpp
#include <iostream>
#include <array>
#include <TypedView/view.h>
using namespace std;
using namespace TypedView;

// Data array with numbers
const array<uint8_t, 8> mArray{
    0xD1, 0x07,  // 2001
    0xD2, 0x07,  // 2002
    0xD3, 0x07,  // 2003
    0xD4, 0x07   // 2004
};

int main()
{
    ViewReader<int8_t,int16_t,int32_t,int64_t,uint8_t,uint16_t,uint32_t,uint64_t> reader(mArray.data(),mArray.size());

    cout << "Read as stream" << endl;
    cout << "Offset: " << reader.GetOffset() << endl;

    {
        auto bytes = reader.ReadAs<uint16_t>();
        cout << hex << "Offset: " << reader.GetOffset() << " result: " << bytes << endl;
    }
    {
        auto bytes = reader.ReadAs<uint16_t>();
        cout << hex << "Offset: " << reader.GetOffset() << " result: " << bytes << endl;
    }
    {
        auto bytes = reader.ReadAs<uint16_t>();
        cout << hex << "Offset: " << reader.GetOffset() << " result: " << bytes << endl;
    }
    {
        auto bytes = reader.ReadAs<uint16_t>();
        cout << hex << "Offset: " << reader.GetOffset() << " result: " << bytes << endl;
    }

    // Set offset to 0
    reader.SetOffset(0);

    cout << "\nRead as 8-byte integer" << endl;

    {
        auto bytes = reader.ReadAs<uint64_t>();
        cout << hex << "Offset: " << reader.GetOffset() << " result: " << bytes << endl;
    }

    cout << "\nRead more than fit in array" << endl;
    try {
        {
            auto bytes = reader.ReadAs<uint64_t>();
            cout << hex << "Offset: " << reader.GetOffset() << " result: " << bytes << endl;
        }
    } catch (std::range_error e) {
        cerr << e.what() << endl;
    }

    // Set offset to 0
    reader.SetOffset(0);

    cout << "\nRead as array of 1 byte integers" << endl;

    {
        auto view = reader.GetViewAs<uint8_t>();

        for (size_t i = 0; i < view.Length(); i++) {
            printf("result: %.2X\n", view[i]);
        }
    }

    cout << "\nRead as array of 2 byte integers" << endl;

    {
        auto view = reader.GetViewAs<uint16_t>();

        for (size_t i = 0; i < view.Length(); i++) {
            printf("result: %.2X\n", view[i]);
        }
    }

    cout << "\nRead as array of 4 byte integers via iterators" << endl;

    {
        auto view = reader.GetViewAs<int32_t>();

        for (auto v : view) {
            printf("result: %.4X\n", v);
        }
    }

    return 0;
}

```
