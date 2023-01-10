#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) {
    _storage.resize(capacity);
    _offset = _starting_offset = 0;
    _readLen = _writeLen = 0;
    _end = false;
}

size_t ByteStream::write(const string &data) {
    size_t writeLen = min(data.size(), remaining_capacity());
    if (writeLen == 0) {
        return writeLen;
    }
    if (_storage.size() - _offset < data.size()) {
        for (size_t i = 0; i < buffer_size(); i++) {
            _storage[i] = _storage[i + _starting_offset];
        }
        _offset -= _starting_offset;
        _starting_offset = 0;
    }
    for (size_t i = 0; i < writeLen; i++) {
        _storage[_offset + i] = data[i];
    }
    _offset += writeLen;
    _writeLen += writeLen;
    return writeLen;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    return {_storage.data() + _starting_offset, min(len, buffer_size())};
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t popLen = min(len, buffer_size());
    _starting_offset += popLen;
    _readLen += popLen;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    size_t readLen = min(len, buffer_size());
    if (readLen == 0) {
        return {};
    }
    string s{_storage.data() + _starting_offset, readLen};
    _starting_offset += readLen;
    _readLen += readLen;
    return s;
}

void ByteStream::end_input() { _end = true; }

bool ByteStream::input_ended() const { return _end; }

size_t ByteStream::buffer_size() const { return _offset - _starting_offset; }

bool ByteStream::buffer_empty() const { return buffer_size() == 0; }

bool ByteStream::eof() const { return input_ended() && buffer_empty(); }

size_t ByteStream::bytes_written() const { return _writeLen; }

size_t ByteStream::bytes_read() const { return _readLen; }

size_t ByteStream::remaining_capacity() const { return _storage.size() - _offset + _starting_offset; }
