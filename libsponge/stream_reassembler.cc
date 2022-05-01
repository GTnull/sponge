#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {
    _buffer.resize(capacity);
}

long StreamReassembler::merge_block(block_node &lhs, const block_node &rhs) {
    //       [***] (***)
    //       [*** (***) ]
    //       [*** (***] ***)
    block_node x, y;
    x = lhs;
    y = rhs;
    if (rhs < lhs) {
        x = rhs;
        y = lhs;
    }
    if (x.begin + x.length < y.begin) {
        return -1;
    } else if (x.begin + x.length >= y.begin + y.length) {
        lhs = x;
        return y.length;
    } else {
        lhs.begin = x.begin;
        lhs.data = x.data + y.data.substr(x.begin + x.length - y.begin);
        lhs.length = lhs.data.length();

        return x.begin + x.length - y.begin;
    }
}
//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // index 实际上是每个byte的序号，而不是每次输入的string的起始序号
    // 所以会有以下3种情况
    //               _head_index
    // ---------------- | ---------------
    //       [***]
    //                [***]
    //                      [***]
    if (index >= _head_index + _capacity) {  // index过大，直接丢弃
        return;
    }

    block_node elm;
    if (index + data.length() <= _head_index) {  // 情况1
        if (eof) {
            _eof_flag = true;
        }
        if (_eof_flag && empty()) {
            _output.end_input();
        }
        return;
    } else if (index < _head_index) {  // 情况2，写入部分bytes
        size_t offset = _head_index - index;
        elm.data.assign(data.begin() + offset, data.end());
        elm.begin = _head_index;
        elm.length = elm.data.length();
    } else {  // 情况3，写入全部bytes
        elm.begin = index;
        elm.length = data.length();
        elm.data = data;
    }
    _unassembled_byte += elm.length;

    // 吞并前后有交叉的字串
    do {
        // merge next
        long merged_bytes = 0;
        auto iter = _blocks.lower_bound(elm);
        while (iter != _blocks.end() && (merged_bytes = merge_block(elm, *iter)) >= 0) {
            _unassembled_byte -= merged_bytes;
            _blocks.erase(iter);
            iter = _blocks.lower_bound(elm);
        }
        // merge prev
        if (iter == _blocks.begin()) {
            break;
        }
        iter--;
        while ((merged_bytes = merge_block(elm, *iter)) >= 0) {
            _unassembled_byte -= merged_bytes;
            _blocks.erase(iter);
            iter = _blocks.lower_bound(elm);
            if (iter == _blocks.begin()) {
                break;
            }
            iter--;
        }
    } while (false);
    _blocks.insert(elm);

    // 情况2，需要右移_head_index，将数据写入到ByteStream
    if (!_blocks.empty() && _blocks.begin()->begin == _head_index) {
        const block_node head_block = *_blocks.begin();
        // modify _head_index and _unassembled_byte according to successful write to _output
        size_t write_bytes = _output.write(head_block.data);
        _head_index += write_bytes;
        _unassembled_byte -= write_bytes;
        _blocks.erase(_blocks.begin());
    }

    if (eof) {
        _eof_flag = true;
    }
    if (_eof_flag && empty()) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_byte; }

bool StreamReassembler::empty() const { return _unassembled_byte == 0; }
