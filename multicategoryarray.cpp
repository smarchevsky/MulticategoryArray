#ifndef MULTICATEGORYARRAY_H
#define MULTICATEGORYARRAY_H

#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <functional>
#include <vector>

#define INDEX_INVALID -1
template <typename ClassType, int MaxCategoryNum>
class MulticategoryArray {
protected:
    std::vector<ClassType> m_itemArray;
    std::array<int, MaxCategoryNum - 1> splits;

    void offsetSplits(int offset, int categoryIndexStart, int categoryIndexEnd = MaxCategoryNum)
    {
        assert(categoryIndexStart >= 0 && categoryIndexStart < MaxCategoryNum);
        if (categoryIndexStart < MaxCategoryNum - 1) {
            if (categoryIndexStart == 0)
                assert(splits.at(0) + offset >= 0);
            else
                assert(splits.at(categoryIndexStart) + offset >= splits.at(categoryIndexStart - 1));

            for (int i = categoryIndexStart; i < categoryIndexEnd - 1; ++i)
                splits.at(i) += offset;
        }
    }

    void modifyData(int newCategoryIndex, const ClassType* newData, int newArrayLength, bool replace)
    {
        assert(newCategoryIndex >= 0 && newCategoryIndex < MaxCategoryNum);
        int separatorPosL = categoryPosL(newCategoryIndex);
        int separatorPosR = categoryPosR(newCategoryIndex);

        int oldArrayLength = separatorPosR - separatorPosL;
        assert(oldArrayLength >= 0);

        int arrayLengthDiff = replace ? newArrayLength - oldArrayLength : newArrayLength;

        if (arrayLengthDiff > 0) {
            m_itemArray.resize(m_itemArray.size() + arrayLengthDiff);
            for (int i = m_itemArray.size() - 1; i >= separatorPosR + arrayLengthDiff; --i)
                m_itemArray[i] = m_itemArray[i - arrayLengthDiff];

        } else if (arrayLengthDiff < 0) {
            for (int i = separatorPosR; i < m_itemArray.size(); ++i)
                m_itemArray[i + arrayLengthDiff] = m_itemArray[i];
            m_itemArray.resize(m_itemArray.size() + arrayLengthDiff);
        }

        if (replace) // start overwrite from left
            for (int i = 0; i < newArrayLength; ++i)
                m_itemArray[i + separatorPosL] = newData[i];
        else // start write from right
            for (int i = 0; i < arrayLengthDiff; ++i)
                m_itemArray[i + separatorPosR] = newData[i];

        offsetSplits(arrayLengthDiff, newCategoryIndex);
    }

public:
    MulticategoryArray() { clear(); }

    void clear()
    {
        m_itemArray.clear();
        memset(splits.data(), 0, sizeof(splits));
    }

    int categoryPosL(int categoryIndex) const
    {
        assert(categoryIndex >= 0 && categoryIndex < MaxCategoryNum);
        return (categoryIndex == 0) ? 0 : splits.at(categoryIndex - 1);
    }

    int categoryPosR(int categoryIndex) const
    {
        assert(categoryIndex >= 0 && categoryIndex < MaxCategoryNum);
        return (categoryIndex == MaxCategoryNum - 1) ? m_itemArray.size() : splits.at(categoryIndex);
    }

    int getItemCategory(int itemIndex, int startCategoryIndex) const
    {
        while (startCategoryIndex < MaxCategoryNum) {
            if (itemIndex < categoryPosR(startCategoryIndex))
                break;
            startCategoryIndex++;
        }
        if (startCategoryIndex == MaxCategoryNum)
            startCategoryIndex = INDEX_INVALID;
        return startCategoryIndex;
    }

    ClassType* moveItemToCategory(int itemIndex, int categoryIndex)
    {
        assert(categoryIndex >= 0 && categoryIndex < MaxCategoryNum);
        if (itemIndex < 0 || itemIndex >= m_itemArray.size())
            return nullptr;

        const int categoryIndexOld = getItemCategory(itemIndex, 0);

        if (categoryIndexOld == INDEX_INVALID)
            return nullptr;

        if (categoryIndex == categoryIndexOld)
            return &m_itemArray[itemIndex];

        if (categoryIndex > categoryIndexOld) {
            offsetSplits(-1, categoryIndexOld, categoryIndex + 1);
            int newCategoryL = categoryPosL(categoryIndex); // move to the left, to reduce swaps
            
            auto tmp = std::move(m_itemArray[itemIndex]);
            for (int i = itemIndex + 1; i <= newCategoryL; ++i)
                m_itemArray[i - 1] = std::move(m_itemArray[i]);
            m_itemArray[newCategoryL] = std::move(tmp);
            
            return &m_itemArray[newCategoryL];
        }

        if (categoryIndex < categoryIndexOld) {
            offsetSplits(1, categoryIndex, categoryIndexOld + 1);
            int newCategoryR = categoryPosR(categoryIndex) - 1; // move to the right, to reduce swaps
            
            auto tmp = std::move(m_itemArray[itemIndex]);
            for (int i = itemIndex - 1; i >= newCategoryR; --i)
                m_itemArray[i + 1] = std::move(m_itemArray[i]);
            m_itemArray[newCategoryR] = std::move(tmp);
            
            return &m_itemArray[newCategoryR];
        }

        return nullptr;
    }

    ClassType* getCategoryStartPtr(int categoryIndex)
    {
        int posL = categoryPosL(categoryIndex);
        int size = categoryPosR(categoryIndex) - posL;
        if (size == 0)
            return nullptr;
        return m_itemArray.data() + posL;
    }

    ClassType* getItemByIndex(int itemIndex) { return &m_itemArray.at(); }
    const ClassType* getItemByIndex(int itemIndex) const { return &m_itemArray.at(); }

    int getItemIndexByPredicate(std::function<bool(const ClassType&)> predicate) const
    {
        for (int i = 0; i < m_itemArray.size(); ++i)
            if (predicate(m_itemArray[i]))
                return i;
        return INDEX_INVALID;
    }

    void forEachItemInCategory(int categoryIndex, std::function<void(const ClassType&)> predicate)
    {
        int posL = categoryPosL(categoryIndex);
        int posR = categoryPosR(categoryIndex);
        for (int i = posL; i < posR; ++i)
            predicate(m_itemArray[i]);
    }

    void forEachItem(std::function<void(const ClassType&)> predicate)
    {
        for (int i = 0; i < m_itemArray.size(); ++i)
            predicate(m_itemArray[i]);
    }

    void printCategorySplits() const
    {
        printf("Splits: ");
        for (int i = 0; i < MaxCategoryNum - 1; ++i)
            printf("%s%d: %d", (i == 0) ? "" : ",  ", i, splits.at(i));
        printf("\n");
    }

    void setItemArray(int categotyIndex, const ClassType* arr, int arrLength) { modifyData(categotyIndex, arr, arrLength, true); }
    void addItemArray(int categoryIndex, const ClassType* arr, int arrLength) { modifyData(categoryIndex, arr, arrLength, false); }
    void addItem(int categoryIndex, const ClassType& item) { modifyData(categoryIndex, &item, 1, false); }

    void removeItem(int itemIndex)
    {
        offsetSplits(-1, getItemCategory(itemIndex, 0));
        m_itemArray.erase(m_itemArray.begin() + itemIndex);
    }

    void removeCategory(int categotyIndex) { setItemArray(categotyIndex, nullptr, 0); }

    constexpr int getCategoriesNum() const { return MaxCategoryNum; }
};

//  0: Reset    1: Red      2: Green    3: Yellow   4: Blue     5: Magenta  6: Cyan   7: Light Gray
static const char* ansiColors[] = {
    "\033[0m", "\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m", "\033[37m"
};

template <int MaxCategoryNum>
class MulticategoryText : public MulticategoryArray<char, MaxCategoryNum> {
public:
    void setText(int categotyIndex, const char* text, bool withNullTerm = false)
    {
        this->modifyData(categotyIndex, text, strlen(text) + (withNullTerm ? 1 : 0), true);
    }

    void addText(int categoryIndex, const char* text, bool withNullTerm = false)
    {
        this->modifyData(categoryIndex, text, strlen(text) + (withNullTerm ? 1 : 0), false);
    }

    void printText() const
    {
        static constexpr int ansiColorsNum = sizeof(ansiColors) / sizeof(ansiColors[0]);

        int categoryIndex = 0;

        printf("%s", ansiColors[categoryIndex % ansiColorsNum]);

        for (int charIndex = 0; charIndex < this->m_itemArray.size(); ++charIndex) {
            int newCategoryIndex = this->getItemCategory(charIndex, categoryIndex);
            if (newCategoryIndex != categoryIndex) {
                categoryIndex = newCategoryIndex;
                printf("%s", ansiColors[categoryIndex % ansiColorsNum]);
            }

            auto currChar = this->m_itemArray[charIndex];
            // clang-format off
                switch (currChar) {
                  case '\0': { printf("\033[30m\\0\033[0m"); } break; // print black \0 and then default
                  default: { putchar(currChar); }
                }
            // clang-format on
        }

        printf("%s", ansiColors[0]);
        printf("   arrayLen: %d", this->m_itemArray.size());
        putchar('\n');
    }
};

#endif // MULTICATEGORYARRAY_H
