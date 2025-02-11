#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : _buffer(), _capacity(capacity), _size(0), _remain(capacity), _nwritten(0), _nread(0), _end_input(false) {}

size_t ByteStream::write(const string &data) {
    size_t _len = min(_remain, data.size());
    for (size_t i = 0; i < _len; i++) {
        _buffer.emplace_back(data[i]);
    }
    _size += _len;
    _remain -= _len;
    _nwritten += _len;
    return _len;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t _len = min(_size, len);
    return string(_buffer.begin(), _buffer.begin() + _len);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t _len = min(_size, len);
    for (size_t i = 0; i < _len; i++) {
        _buffer.pop_front();
    }
    _size -= _len;
    _remain += _len;
    _nread += _len;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string ret = peek_output(len);
    pop_output(len);
    return ret;
}

void ByteStream::end_input() { _end_input = true; }

bool ByteStream::input_ended() const { return _end_input; }

size_t ByteStream::buffer_size() const { return _size; }

bool ByteStream::buffer_empty() const { return _size == 0; }

bool ByteStream::eof() const { return input_ended() && buffer_empty(); }

size_t ByteStream::bytes_written() const { return _nwritten; }

size_t ByteStream::bytes_read() const { return _nread; }

size_t ByteStream::remaining_capacity() const { return _remain; }
