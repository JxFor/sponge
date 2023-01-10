#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity)
    ,_end(false){}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    //当前已经合并的位置
    size_t dataWriteLen = 0;//data可以写的长度
    size_t dataWriteIndex = 0;
    size_t writeIndex = _output.bytes_written();
    size_t startIndex = _output.bytes_read();
    size_t maxIndex = startIndex + _capacity;

    if( eof && index == writeIndex && data.empty() ){
        _output.end_input();
        _end = true;
        return;
    }
    if( data.size() + index <= writeIndex){
        //已经处理过的消息，丢弃
        return;
    }

    if( index <= writeIndex){
        //可以直接添加到output
        dataWriteIndex = writeIndex - index;
        if( dataWriteIndex >= data.size()){
            return;
        }
        dataWriteLen = min(data.size() - dataWriteIndex,maxIndex-writeIndex);
        if( dataWriteLen <= 0 ){
            return;
        }
        _output.write(data.substr(dataWriteIndex,dataWriteIndex+dataWriteLen));
        writeIndex = _output.bytes_written();
        //data 无法写满的，忽略eof
        if( eof && ((dataWriteLen+dataWriteIndex) == data.size())){
            _end = true;
        }
        //unassembled 合并检查
        auto iter = _unAssembled.begin();
        for (; iter != _unAssembled.end();){
            //检测
            size_t unIndex = iter->first;
            if( unIndex + iter->second.size() <= writeIndex){
                iter = _unAssembled.erase(iter);
                continue;
            }else if( unIndex > writeIndex){
                break;
            }
            //可以合并
            _output.write(iter->second.substr(writeIndex - unIndex));
            writeIndex = _output.bytes_written();
            iter = _unAssembled.erase(iter);
        }
    }else{
        auto iter = _unAssembled.begin();
        for (; iter != _unAssembled.end();iter++){
            size_t unIndex = iter->first;
            if( index == unIndex ){
                if( data.size() > iter->second.size()){
                    dataWriteLen = min(data.size(),maxIndex-index);
                    _unAssembled[unIndex] = data.substr(0, dataWriteLen);
                }
                break;
            }
        }
        if( iter == _unAssembled.end() ){
            //添加到尾部，超过容量的丢弃
            if( index  >= maxIndex ){
                return;
            }
            dataWriteLen = min(data.size(),maxIndex-index);
            if( eof && ((dataWriteLen+dataWriteIndex) == data.size())){
                _end = true;
            }
            if( dataWriteLen > 0 ) {
                _unAssembled.insert(make_pair(index, data.substr(0, dataWriteLen)));
            }
        }
        if(_unAssembled.empty()){
            if(_end){
                _output.end_input();
            }
            return;
        }
        //从第二个开始比较
        iter = _unAssembled.begin();
        size_t lastIndex = iter->first;
        iter++;
        for (; iter != _unAssembled.end();){
            if( iter->first > (lastIndex+_unAssembled[lastIndex].size())){
                lastIndex = iter->first;
                iter++;
                continue;
            }
            if( lastIndex + _unAssembled[lastIndex].size() >= (iter->first+iter->second.size())){
                //上一个包含这个
                iter = _unAssembled.erase(iter);
            }else{
                //上一个添加这个
                _unAssembled[lastIndex].append( iter->second.substr(lastIndex+_unAssembled[lastIndex].size()-iter->first));
                iter = _unAssembled.erase(iter);
            }
        }
    }
    if( _unAssembled.empty() && _end){
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const {
    auto iter = _unAssembled.begin();
    size_t len = 0;
    for (; iter != _unAssembled.end();iter++){
        len += iter->second.size();
    }
    return len;
}

bool StreamReassembler::empty() const { return _end && _output.buffer_empty() && _unAssembled.empty(); }