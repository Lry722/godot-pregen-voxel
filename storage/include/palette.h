#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <format>
#include <memory>

template <typename TIndex, typename TData>
class Palette
{
public:
    struct Entry
    {
        TIndex index;
        TData data;
        // 最坏情况下，每个Entry的ref都为1，因此index和ref要为同类型
        TIndex ref{0};
    };
    typedef TIndex IndexType;
    typedef TData DataType;

public:
    // 添加数据，返回对应的Entry的index
    TIndex add(const TData data);
    // 移除数据，返回对应的Entry的index
    TIndex remove(const TData data);

    // 选择index对应的data
    TData pick(const TIndex index) const;
    // 选择data对应的index
    TIndex indexOf(const TData data) const;

    // 删除ref为0的entry
    void fit();

    // 序列化
    void serialize(std::ostringstream & oss);
    // 反序列化
    void deserialize(std::istringstream & iss, const size_t size);

private:
    std::vector<Entry> entries;
};

#include "palette.inl"