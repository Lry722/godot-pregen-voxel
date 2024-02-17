#pragma once
#include "palette.h"
#include <cstring>
#include <sstream>

template <typename TIndex, typename TData>
TIndex Palette<TIndex, TData>::add(const TData data)
{
    // 为空时直接插入
    if (entries.size() == 0)
    {
        entries.emplace_back(0, data, 1);
        return 0;
    }

    // 二分查找第一个不小于data的元素
    auto iter = std::lower_bound(entries.begin(), entries.end(), data, [](const Entry &a, const TData &b)
                                 { return a.data < b; });
    if (iter == entries.end() || iter->data != data)
        // 如果当前不存在含有data的Entry，则插入一条
        iter = entries.emplace(iter, (iter - 1)->index + 1, data);

    ++(iter->ref);

    return iter->index;
}

template <typename TIndex, typename TData>
TData Palette<TIndex, TData>::pick(const TIndex index) const
{
    auto iter = std::lower_bound(entries.begin(), entries.end(), index, [](const Entry &a, const TIndex &b)
                                 { return a.index < b; });
    if (iter == entries.end()) [[unlikely]]
        throw std::runtime_error(std::format("Palette: Pick non-existent index {}.", index));

    return iter->data;
}

template <typename TIndex, typename TData>
TIndex Palette<TIndex, TData>::indexOf(const TData data) const
{
    auto iter = std::lower_bound(entries.begin(), entries.end(), data, [](const Entry &a, const Entry &b)
                                 { return a.data < b.data; });
    if (iter == entries.end()) [[unlikely]]
        throw std::runtime_error(std::format("Palette: Pick non-existent index {}.", data));

    return iter->index;
}

template <typename TIndex, typename TData>
void Palette<TIndex, TData>::fit()
{
    if (entries.empty())
        return;

    entries.erase(
        std::remove_if(entries.begin(), entries.end(),
                       [](const Entry &entry)
                       { return entry.ref == 0; }),
        entries.end());
}

template <typename TIndex, typename TData>
TIndex Palette<TIndex, TData>::remove(const TData data)
{
    auto iter = std::lower_bound(entries.begin(), entries.end(), data, [](const Entry &a, const Entry &b)
                                 { return a.data < b.data; });
    if (iter == entries.end() || iter->ref == 0) [[unlikely]]
        throw std::runtime_error(std::format("Palette: Remove non-existent data {}.", data));
    --(iter->ref);

    return iter->index;
}

template <typename TIndex, typename TData>
void Palette<TIndex, TData>::serialize(std::ostringstream & oss)
{
    fit();
    oss.write(reinterpret_cast<char *>(entries.data()), entries.size() * sizeof(Entry));
}

template <typename TIndex, typename TData>
void Palette<TIndex, TData>::deserialize(std::istringstream & iss, const size_t size)
{
    entries.resize(size / sizeof(Entry));
    iss.read(reinterpret_cast<char *>(entries.data()), size * sizeof(Entry));
}
