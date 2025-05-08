#ifndef MULTIGROUPARRAY_H
#define MULTIGROUPARRAY_H

#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <functional>
#include <vector>

#define INDEX_INVALID -1
template <typename ClassType, int MaxGroupNum>
class MultiGroupArray {
protected:
    std::vector<ClassType> m_itemArray;
    std::array<int, MaxGroupNum - 1> m_splits;

    void offsetSplits(int offset, int groupIndexStart, int groupIndexEnd = MaxGroupNum)
    {
        assert(groupIndexStart >= 0 && groupIndexStart < MaxGroupNum);
        if (groupIndexStart < MaxGroupNum - 1) {
            if (groupIndexStart == 0)
                assert(m_splits.at(0) + offset >= 0);
            else
                assert(m_splits.at(groupIndexStart) + offset >= m_splits.at(groupIndexStart - 1));

            for (int i = groupIndexStart; i < groupIndexEnd - 1; ++i)
                m_splits.at(i) += offset;
        }
    }

    void modifyData(int newGroupIndex, const ClassType* newData, int newArrayLength, bool replace)
    {
        assert(newGroupIndex >= 0 && newGroupIndex < MaxGroupNum);
        int separatorPosL = groupPosL(newGroupIndex);
        int separatorPosR = groupPosR(newGroupIndex);

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

        offsetSplits(arrayLengthDiff, newGroupIndex);
    }

public:
    MultiGroupArray() { clear(); }

    void clear()
    {
        m_itemArray.clear();
        memset(m_splits.data(), 0, sizeof(m_splits));
    }

    int groupPosL(int groupIndex) const
    {
        assert(groupIndex >= 0 && groupIndex < MaxGroupNum);
        return (groupIndex == 0) ? 0 : m_splits.at(groupIndex - 1);
    }

    int groupPosR(int groupIndex) const
    {
        assert(groupIndex >= 0 && groupIndex < MaxGroupNum);
        return (groupIndex == MaxGroupNum - 1) ? m_itemArray.size() : m_splits.at(groupIndex);
    }

    int getItemGroup(int itemIndex, int startGroupIndex) const
    {
        while (startGroupIndex < MaxGroupNum) {
            if (itemIndex < groupPosR(startGroupIndex))
                break;
            startGroupIndex++;
        }
        if (startGroupIndex == MaxGroupNum)
            startGroupIndex = INDEX_INVALID;
        return startGroupIndex;
    }

    ClassType* moveItemToGroup(int itemIndex, int groupIndex)
    {
        assert(groupIndex >= 0 && groupIndex < MaxGroupNum);
        if (itemIndex < 0 || itemIndex >= m_itemArray.size())
            return nullptr;

        const int groupIndexOld = getItemGroup(itemIndex, 0);

        if (groupIndexOld == INDEX_INVALID)
            return nullptr;

        if (groupIndex == groupIndexOld)
            return &m_itemArray[itemIndex];

        if (groupIndex > groupIndexOld) {
            offsetSplits(-1, groupIndexOld, groupIndex + 1);
            int newGroupL = groupPosL(groupIndex); // move to the left, to reduce swaps

            auto tmp = std::move(m_itemArray[itemIndex]);
            for (int i = itemIndex + 1; i <= newGroupL; ++i)
                m_itemArray[i - 1] = std::move(m_itemArray[i]);
            m_itemArray[newGroupL] = std::move(tmp);

            return &m_itemArray[newGroupL];
        }

        if (groupIndex < groupIndexOld) {
            offsetSplits(1, groupIndex, groupIndexOld + 1);
            int newGroupR = groupPosR(groupIndex) - 1; // move to the right, to reduce swaps

            auto tmp = std::move(m_itemArray[itemIndex]);
            for (int i = itemIndex - 1; i >= newGroupR; --i)
                m_itemArray[i + 1] = std::move(m_itemArray[i]);
            m_itemArray[newGroupR] = std::move(tmp);

            return &m_itemArray[newGroupR];
        }

        return nullptr;
    }

    ClassType* getGroupStartPtr(int groupIndex)
    {
        int posL = groupPosL(groupIndex);
        int size = groupPosR(groupIndex) - posL;
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

    void forEachItemInGroup(int groupIndex, std::function<void(const ClassType&)> predicate)
    {
        int posL = groupPosL(groupIndex);
        int posR = groupPosR(groupIndex);
        for (int i = posL; i < posR; ++i)
            predicate(m_itemArray[i]);
    }

    void forEachItem(std::function<void(const ClassType&)> predicate)
    {
        for (int i = 0; i < m_itemArray.size(); ++i)
            predicate(m_itemArray[i]);
    }

    void printGroupSplits() const
    {
        printf("Splits: ");
        for (int i = 0; i < MaxGroupNum - 1; ++i)
            printf("%s%d: %d", (i == 0) ? "" : ",  ", i, m_splits.at(i));
        printf("\n");
    }

    void setItemArray(int groupIndex, const ClassType* arr, int arrLength) { modifyData(groupIndex, arr, arrLength, true); }
    void addItemArray(int groupIndex, const ClassType* arr, int arrLength) { modifyData(groupIndex, arr, arrLength, false); }
    void addItem(int groupIndex, const ClassType& item) { modifyData(groupIndex, &item, 1, false); }

    void removeItem(int itemIndex)
    {
        offsetSplits(-1, getItemGroup(itemIndex, 0));
        m_itemArray.erase(m_itemArray.begin() + itemIndex);
    }

    void removeGroup(int groupIndex) { setItemArray(groupIndex, nullptr, 0); }

    constexpr int getCategoriesNum() const { return MaxGroupNum; }
};

//  0: Reset    1: Red      2: Green    3: Yellow   4: Blue     5: Magenta  6: Cyan   7: Light Gray
static const char* ansiColors[] = {
    "\033[0m", "\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m", "\033[37m"
};

template <int MaxGroupNum>
class MultiGroupText : public MultiGroupArray<char, MaxGroupNum> {
public:
    void setText(int groupIndex, const char* text, bool withNullTerm = false)
    {
        this->modifyData(groupIndex, text, strlen(text) + (withNullTerm ? 1 : 0), true);
    }

    void addText(int groupIndex, const char* text, bool withNullTerm = false)
    {
        this->modifyData(groupIndex, text, strlen(text) + (withNullTerm ? 1 : 0), false);
    }

    void printText() const
    {
        static constexpr int ansiColorsNum = sizeof(ansiColors) / sizeof(ansiColors[0]);

        int groupIndex = 0;

        printf("%s", ansiColors[groupIndex % ansiColorsNum]);

        for (int charIndex = 0; charIndex < this->m_itemArray.size(); ++charIndex) {
            int newGroupIndex = this->getItemGroup(charIndex, groupIndex);
            if (newGroupIndex != groupIndex) {
                groupIndex = newGroupIndex;
                printf("%s", ansiColors[groupIndex % ansiColorsNum]);
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
