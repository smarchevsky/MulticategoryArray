# Multicategory array

This thing is created to keep multiple arrays of the same type in a contiguous block.
<br>Done in a hurry, beautification later.

Example of char array:
```cpp
MulticategoryText<8> mcArray; // is MulticategoryArray<char, 8>
mcArray.addText(0, "data_array_one");
mcArray.addText(1, "data_array_two");
mcArray.addText(2, "data_array_three");
mcArray.addText(3, "data_array_four");

0          split 0       split 1         split 2      array.size()
|             |             |               |              |
data_array_onedata_array_twodata_array_threedata_array_four
```
`MulticategoryText<MaxCategoryNum>` is inherited `MulticategoryArray<char, MaxCategoryNum>`
<br>`addText(..)` is `addItemArray(..)`, adds new array to the end of corresponding category
<br>But `addText(..)` length is specified by `strlen` (`\0`).


You can add text with null terminator, so each category text can bre read as separate string:
```cpp
mcArray.addText(0, "data_array_one", true); // add array, len is specified by \0 (strlen)
mcArray.addText(1, "data_array_two", true);
mcArray.addText(2, "data_array_three", true);
mcArray.addText(3, "data_array_four", true);

0            split 0         split 1           split 2        array.size()
|               |               |                 |                |
data_array_one\0data_array_two\0data_array_three\0data_array_four\0
```
`setText(..)` is `setItemArray(..)`, the same, but replace items in the same category.
<br>`setItemArray(..)` with 0 size array removes category (replaces current category with nothing).

```cpp
mcArray.addText(0, "ABCD"); // ABCD
mcArray.addText(1, "EFGH"); // ABCD EFGH
mcArray.setText(0, "1");    // 1 EFGH
```
Get index, get category, remove item:
```cpp
mcArray.addText(0, "ABCD");
mcArray.addText(1, "EFGH");
mcArray.addText(2, "IJKL");

// find item and category
int indexH = mcArray.getItemIndexByPredicate([&](const char& c) { return c == 'H'; }); // 7
int categoryH = mcArray.getItemCategory(indexH, 0); // category 1

// remove item
mcArray.removeItem(indexH); // ABCD EFG IJKL
indexH = mcArray.getItemIndexByPredicate([&](const char& c) { return c == 'H'; }); // -1
```
Move F to the different categories. Array is `ABCD EFG IJKL` now.
```cpp
int indexF = mcArray.getItemIndexByPredicate([&](const char& c) { return c == 'F'; }); // 5
int categoryF = mcArray.getItemCategory(indexF, 0); // category 1

if(moveToCategory2)
  mcArray.moveItemToCategory(indexF, 2); // ABCD EG FIJKL moves to the left of ascend category
else if(moveToCategory0)
  mcArray.moveItemToCategory(indexF, 0); // ABCDF EG IJKL moves to the right of desc category
else
  mcArray.moveItemToCategory(indexF, 1); // ABCD EFG IJKL (nothing happens, same category)
```
`MulticategoryText::printText()` prints text with ANSI colors.

$${\color{#aaaaaa}ABCD\color{#ee0000}EFGH\color{#00cc00}IJKL\color{#bbbb00}MNOP\color{#6666ff}QRST\color{#cc00cc}UVWX\color{#00cccc}YZ}$$

Oh my garble Danila, what an aesthetic pleasure! *(if color markdown is visible)*

```cpp
int indexL = mcArray.getItemIndexByPredicate([&](const char& c) { return c == 'L'; });
mcArray.moveItemToCategory(indexL, 5); // move L from 2 category to 5
```
$${\color{#aaaaaa}ABCD\color{#ee0000}EFGH\color{#00cc00}IJK\color{#bbbb00}MNOP\color{#6666ff}QRST\color{#cc00cc}LUVWX\color{#00cccc}YZ}$$


Works not only with texts, but texts are easier to debug.<br>That's  all, you are welcome!
