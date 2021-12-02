#pragma once

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <limits>
#include <memory>
#include <vector>
#include <string>


//#define log_bytes_rw
/** template classes specifying the storage used. this is needed because std::vector<std::byte, std::allocator<std::byte>> and std::vector<std::byte, some_custom_allocator>
are seen as different types by the compiler, so it wouldn't be possible to write/read to a 4 byte aligned buffer, for example,
using std::vector<std::byte> as base for storage(std::vector<std::byte> is 1 byte aligned by default).
though templated, byte_seqv is expected to have the same api as std::vector(eg. data(), size(), operator[], etc)
*/
template<class byte_seqv> class data_seqv_tpl;
template<class byte_seqv> class data_seqv_ro_mem_tpl;
template<class byte_seqv> class data_seqv_rw_mem_tpl;
template<class T, class io, class byte_seqv> class data_seqv_file_base_tpl;
template<class byte_seqv> class data_seqv_file_tpl;
template<class byte_seqv> class data_seqv_file_ptr_tpl;
template<class byte_seqv> class data_seqv_file_shr_tpl;
template<class T, class reader> class data_seqv_reader_base_tpl;
template<class byte_seqv> class data_seqv_ro_mem_reader_tpl;
template<class byte_seqv> class data_seqv_ro_mem_reader_ref_tpl;
template<class byte_seqv> class data_seqv_mem_reader_ref_tpl;
template<class byte_seqv> class data_seqv_file_reader_ref_tpl;
template<class byte_seqv> class data_seqv_reader_ptr_tpl;
template<class byte_seqv> class data_seqv_reader_shr_tpl;
template<class T, class writer> class data_seqv_writer_base_tpl;
template<class byte_seqv> class data_seqv_mem_writer_tpl;
template<class byte_seqv> class data_seqv_mem_writer_ref_tpl;
template<class byte_seqv> class data_seqv_file_writer_ref_tpl;
template<class byte_seqv> class data_seqv_writer_ptr_tpl;
template<class byte_seqv> class data_seqv_writer_shr_tpl;
template<class byte_seqv> class data_seqv_rw_mem_ops_tpl;
template<class byte_seqv> class data_seqv_rw_file_ops_tpl;
template<class byte_seqv> class data_seqv_rw_ops_ptr_tpl;
template<class byte_seqv> class data_seqv_rw_ops_shr_tpl;

// default to using std::vector<std::byte> as storage(byte_seqv) for the above template classes
using byte_vect = std::vector<std::byte>;
using data_seqv = data_seqv_tpl<byte_vect>;
using data_seqv_ro_mem = data_seqv_ro_mem_tpl<byte_vect>;
using data_seqv_rw_mem = data_seqv_rw_mem_tpl<byte_vect>;
template<class T, class io> using data_seqv_file_base = data_seqv_file_base_tpl<T, io, byte_vect>;
using data_seqv_file = data_seqv_file_tpl<byte_vect>;
using data_seqv_file_ptr = data_seqv_file_ptr_tpl<byte_vect>;
using data_seqv_file_shr = data_seqv_file_shr_tpl<byte_vect>;
using data_seqv_ro_mem_reader = data_seqv_ro_mem_reader_tpl<byte_vect>;
using data_seqv_ro_mem_reader_ref = data_seqv_ro_mem_reader_ref_tpl<byte_vect>;
using data_seqv_mem_reader_ref = data_seqv_mem_reader_ref_tpl<byte_vect>;
using data_seqv_file_reader_ref = data_seqv_file_reader_ref_tpl<byte_vect>;
using data_seqv_reader_ptr = data_seqv_reader_ptr_tpl<byte_vect>;
using data_seqv_reader_shr = data_seqv_reader_shr_tpl<byte_vect>;
using data_seqv_mem_writer = data_seqv_mem_writer_tpl<byte_vect>;
using data_seqv_mem_writer_ref = data_seqv_mem_writer_ref_tpl<byte_vect>;
using data_seqv_file_writer_ref = data_seqv_file_writer_ref_tpl<byte_vect>;
using data_seqv_writer_ptr = data_seqv_writer_ptr_tpl<byte_vect>;
using data_seqv_writer_shr = data_seqv_writer_shr_tpl<byte_vect>;
using data_seqv_rw_mem_ops = data_seqv_rw_mem_ops_tpl<byte_vect>;
using data_seqv_rw_file_ops = data_seqv_rw_file_ops_tpl<byte_vect>;
using data_seqv_rw_ops_ptr = data_seqv_rw_ops_ptr_tpl<byte_vect>;
using data_seqv_rw_ops_shr = data_seqv_rw_ops_shr_tpl<byte_vect>;


// type conversion utils: type aliasing/punning and reference/pointer converters
template<class T> std::byte* byte_cast(T* i_addr) { return reinterpret_cast<std::byte*>(i_addr); }
template<class T> const std::byte* byte_const_cast(const T* i_addr) { return reinterpret_cast<const std::byte*>(i_addr); }
template<class T> struct ptr_adapter { T operator() (T& i_obj) { return i_obj; } const T operator() (const T& i_obj) { return i_obj; } };
template<class T> struct ref_adapter { T* operator() (T& i_obj) { return &i_obj; } const T* operator() (const T& i_obj) { return &i_obj; } };


/** interface/base class for all data sequences. note: writing to the sequence does not change the read position and viceversa */
template<class byte_seqv> class data_seqv_tpl
{
public:
   data_seqv_tpl();
   data_seqv_tpl(const data_seqv_tpl& i_seqv) { *this = i_seqv; }
   data_seqv_tpl(data_seqv_tpl&& i_seqv) noexcept { *this = std::move(i_seqv); }
   virtual ~data_seqv_tpl() {}
   data_seqv_tpl& operator=(const data_seqv_tpl& i_seqv);
   data_seqv_tpl& operator=(data_seqv_tpl&& i_seqv) noexcept;
   /** returns true when there are no more bytes to read */
   virtual bool is_end_of_seqv();
   virtual void close();
   /** returns the sequence data as a byte array */
   virtual const std::byte* seqv_as_array() const = 0;
   /** returns the sequence data as a byte vector */
   virtual byte_seqv seqv_as_vector() = 0;
   /** returns true if size if 0 */
   virtual bool empty() const { return size() == 0; }
   /** returns total number of bytes in this sequence */
   virtual uint64_t size() const = 0;
   /** current read position */
   uint64_t read_position() const;
   /** current writing position */
   uint64_t write_position() const;
   uint64_t total_bytes_read() const;
   uint64_t total_bytes_written() const;
   virtual bool is_readable() const { return true; }
   virtual bool is_writable() const { return true; }
   /** sets current reading & writing position to 0, but keeps the current size and what was written so far */
   virtual void rewind();
   /** same as rewind, but also discards what was written so far in memory sequences, so afterwards size() will return 0 */
   virtual void reset();
   /** sets current reading & writing position */
   virtual void set_io_position(uint64_t i_position);
   virtual void set_read_position(uint64_t i_position);
   virtual void set_write_position(uint64_t i_position);
   /** returns number of bytes read */
   int read_bytes(std::byte* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   /** returns number of bytes written */
   int write_bytes(const std::byte* i_seqv, size_t i_elem_count, size_t i_offset = 0);

protected:
   /** returns number of bytes read */
   virtual int read_bytes_impl(std::byte* i_seqv, size_t i_elem_count, size_t i_offset) = 0;
   /** returns number of bytes written */
   virtual int write_bytes_impl(const std::byte* i_seqv, size_t i_elem_count, size_t i_offset) = 0;

   uint64_t read_position_v;
   uint64_t write_position_v;
#ifdef log_bytes_rw
   uint64_t total_bytes_read_v;
   uint64_t total_bytes_written_v;
#endif
};


/** read only memory data sequence. doesn't copy the input data, but only stores a reference. be very careful with this.
note: writing to the sequence does not change the read position and viceversa */
template<class byte_seqv> class data_seqv_ro_mem_tpl : public data_seqv_tpl<byte_seqv>
{
public:
   /** constructs an empty ro memory data sequence */
   data_seqv_ro_mem_tpl() {}
   /** constructs a ro memory data sequence with a pointer to data copied from i_seqv. i_seqv's size must NOT be modified while referenced by data_seqv_ro_mem_tpl */
   data_seqv_ro_mem_tpl(const data_seqv_ro_mem_tpl& i_seqv) : data_seqv_tpl<byte_seqv>(i_seqv), seqv_v(i_seqv.seqv_as_array()), size_v(i_seqv.size()) {}
   /** constructs a ro memory data sequence with a pointer to data copied from i_seqv. i_seqv's size must NOT be modified while referenced by data_seqv_ro_mem_tpl */
   data_seqv_ro_mem_tpl(const data_seqv_tpl<byte_seqv>& i_seqv) : seqv_v(i_seqv.seqv_as_array()), size_v(i_seqv.size()) {}
   /** constructs a ro memory data sequence with a pointer to data copied from i_seqv. i_seqv's size must NOT be modified while referenced by data_seqv_ro_mem_tpl */
   data_seqv_ro_mem_tpl(const byte_seqv& i_seqv);
   /** constructs a ro memory data sequence with a pointer to data copied from i_seqv. i_seqv's size must NOT be modified while referenced by data_seqv_ro_mem_tpl */
   data_seqv_ro_mem_tpl(const std::byte* i_seqv, uint64_t i_elem_count);
   virtual ~data_seqv_ro_mem_tpl() {}
   virtual const std::byte* seqv_as_array() const override;
   virtual byte_seqv seqv_as_vector() override;
   virtual uint64_t size() const override;
   virtual bool is_writable() const { return false; }
   virtual void rewind() override;
   virtual void reset() override;
   virtual void set_io_position(uint64_t i_position) override;
   virtual void set_read_position(uint64_t i_position) override;
   virtual void set_write_position(uint64_t i_position) override;

protected:
   virtual int read_bytes_impl(std::byte* i_seqv, size_t i_elem_count, size_t i_offset) override;
   virtual int write_bytes_impl(const std::byte* i_seqv, size_t i_elem_count, size_t i_offset) override;

   /** size of this sequence */
   uint64_t size_v = 0;
   /** note: reference only, does not own the memory that it points to */
   const std::byte* seqv_v = nullptr;
};


/** read/write memory data sequence. note: writing to the sequence does not change the read position and viceversa */
template<class byte_seqv> class data_seqv_rw_mem_tpl : public data_seqv_tpl<byte_seqv>
{
public:
   /** constructs an empty memory data sequence */
   data_seqv_rw_mem_tpl() {}
   /** constructs a memory data sequence with its size the specified number of bytes */
   data_seqv_rw_mem_tpl(size_t i_elem_count) : seqv_v(i_elem_count) {}
   /** constructs a memory data sequence with data copied from i_seqv */
   data_seqv_rw_mem_tpl(const std::byte* i_seqv, size_t i_elem_count);
   /** constructs a memory data sequence with data copied from i_seqv */
   data_seqv_rw_mem_tpl(const data_seqv_rw_mem_tpl& i_seqv) : data_seqv_tpl<byte_seqv>(i_seqv), seqv_v(i_seqv.seqv_v) {}
   /** constructs a memory data sequence with data copied from i_seqv */
   data_seqv_rw_mem_tpl(const byte_seqv& i_seqv) { seqv_v = i_seqv; }
   /** constructs a memory data sequence with data moved from i_seqv */
   data_seqv_rw_mem_tpl(data_seqv_rw_mem_tpl&& i_seqv) noexcept { *this = std::move(i_seqv); }
   /** constructs a memory data sequence with data moved from i_seqv */
   data_seqv_rw_mem_tpl(byte_seqv&& i_seqv) { *this = std::move(i_seqv); }
   virtual ~data_seqv_rw_mem_tpl() {}
   data_seqv_rw_mem_tpl& operator=(const data_seqv_rw_mem_tpl& i_seqv);
   data_seqv_rw_mem_tpl& operator=(const byte_seqv& i_seqv);
   data_seqv_rw_mem_tpl& operator=(data_seqv_rw_mem_tpl&& i_seqv) noexcept;
   data_seqv_rw_mem_tpl& operator=(byte_seqv&& i_seqv) noexcept;
   virtual uint64_t size() const override;
   virtual void rewind() override;
   virtual void reset() override;
   virtual const std::byte* seqv_as_array() const override;
   virtual byte_seqv seqv_as_vector() override;
   virtual void set_io_position(uint64_t i_position) override;
   virtual void set_read_position(uint64_t i_position) override;
   virtual void set_write_position(uint64_t i_position) override;
   /** resizes this sequence to the specified number of bytes */
   virtual void resize(size_t i_elem_count);
   /** moves the data in this sequence into the byte vector i_seqv. this object is left empty */
   virtual void move_into(byte_seqv& i_seqv);

protected:
   virtual int read_bytes_impl(std::byte* i_seqv, size_t i_elem_count, size_t i_offset) override;
   virtual int write_bytes_impl(const std::byte* i_seqv, size_t i_elem_count, size_t i_offset) override;

   byte_seqv seqv_v;
};


// data_seqv_file_base_tpl
/** interface/base class for a wrapper around a file object. note: writing to the sequence does not change the read position and viceversa */
class data_seqv_file_wrapper
{
public:
   virtual ~data_seqv_file_wrapper() {}
   virtual bool is_open() const = 0;
   virtual bool is_writable() const = 0;
   virtual uint64_t length() const = 0;
   virtual void close() = 0;
   virtual void set_io_position(uint64_t i_position) = 0;
   virtual int read_bytes(std::byte* i_seqv, size_t i_size, size_t i_offset = 0) = 0;
   virtual int write_bytes(const std::byte* i_seqv, size_t i_size, size_t i_offset = 0) = 0;
};


/** simple implementation of a file wrapper class using the C file API. note: writing to the sequence does not change the read position and viceversa */
class data_seqv_std_file_wrapper : public data_seqv_file_wrapper
{
public:
   data_seqv_std_file_wrapper();
   data_seqv_std_file_wrapper(std::shared_ptr<std::FILE> i_file, bool i_is_writable);
   data_seqv_std_file_wrapper(const std::string& i_file_path, const std::string& i_open_mode);
   virtual bool is_open() const override;
   virtual bool is_writable() const override;
   virtual uint64_t length() const override;
   virtual void close() override;
   virtual void set_io_position(uint64_t i_position) override;
   virtual int read_bytes(std::byte* i_seqv, size_t i_elem_count, size_t i_offset = 0) override;
   virtual int write_bytes(const std::byte* i_seqv, size_t i_elem_count, size_t i_offset = 0) override;
   virtual std::FILE* file_ptr() const;

protected:
   std::shared_ptr<std::FILE> file_v;
   bool is_writable_v;
};


/** base class for a file data sequence. note: writing to the sequence does not change the read position and viceversa */
template<class T, class io, class byte_seqv> class data_seqv_file_base_tpl : public data_seqv_tpl<byte_seqv>
{
public:
   data_seqv_file_base_tpl(T i_file) : file_v(i_file) {}
   virtual ~data_seqv_file_base_tpl() {}
   virtual bool is_end_of_seqv() override;
   virtual void close() override;
   virtual const std::byte* seqv_as_array() const override { return nullptr; }
   virtual byte_seqv seqv_as_vector() override;
   virtual uint64_t size() const override;
   virtual bool is_writable() const override { return io()(file_v)->is_writable(); }
   virtual void rewind() override;
   virtual void reset() override;
   virtual void set_io_position(uint64_t i_position) override;
   virtual void set_read_position(uint64_t i_position) override;
   virtual void set_write_position(uint64_t i_position) override;
   virtual const T& file() const;

protected:
   virtual int read_bytes_impl(std::byte* i_seqv, size_t i_elem_count, size_t i_offset) override;
   virtual int write_bytes_impl(const std::byte* i_seqv, size_t i_elem_count, size_t i_offset) override;

   uint64_t last_file_pos = 0;
   T file_v;
};


/** file sequence reference version */
template<class byte_seqv> class data_seqv_file_tpl : public data_seqv_file_base_tpl<data_seqv_std_file_wrapper, ref_adapter<data_seqv_std_file_wrapper>, byte_seqv>
{
public:
   data_seqv_file_tpl(const data_seqv_std_file_wrapper& i_file) :
      data_seqv_file_base_tpl<data_seqv_std_file_wrapper, ref_adapter<data_seqv_std_file_wrapper>, byte_seqv>(i_file)
   {
      assert(i_file.is_open());
   }
   virtual void set_file_wrapper(const data_seqv_std_file_wrapper& i_file) { this->file_v = i_file; }
};


/** file sequence pointer version */
template<class byte_seqv> class data_seqv_file_ptr_tpl : public data_seqv_file_base_tpl<data_seqv_file_wrapper*, ptr_adapter<data_seqv_file_wrapper*>, byte_seqv>
{
public:
   data_seqv_file_ptr_tpl() : data_seqv_file_base_tpl<data_seqv_file_wrapper*, ptr_adapter<data_seqv_file_wrapper*>, byte_seqv>(nullptr) {}
   data_seqv_file_ptr_tpl(data_seqv_file_wrapper* i_file) :
      data_seqv_file_base_tpl<data_seqv_file_wrapper*, ptr_adapter<data_seqv_file_wrapper*>, byte_seqv>(i_file) { assert(!i_file || i_file->is_open()); }
   virtual void set_file_wrapper(data_seqv_file_wrapper* i_file) { this->file_v = i_file; }
};


/** file sequence shared pointer version */
template<class byte_seqv> class data_seqv_file_shr_tpl :
   public data_seqv_file_base_tpl<std::shared_ptr<data_seqv_file_wrapper>, ptr_adapter<std::shared_ptr<data_seqv_file_wrapper>>, byte_seqv>
{
public:
   data_seqv_file_shr_tpl() :
      data_seqv_file_base_tpl<std::shared_ptr<data_seqv_file_wrapper>, ptr_adapter<std::shared_ptr<data_seqv_file_wrapper>>, byte_seqv>(nullptr) {}
   data_seqv_file_shr_tpl(std::shared_ptr<data_seqv_file_wrapper> i_file) :
      data_seqv_file_base_tpl<std::shared_ptr<data_seqv_file_wrapper>, ptr_adapter<std::shared_ptr<data_seqv_file_wrapper>>, byte_seqv>(i_file)
   {
      assert(!i_file || i_file->is_open());
   }
   virtual void set_file_wrapper(std::shared_ptr<data_seqv_file_wrapper> i_file) { this->file_v = i_file; }
};


/** data_seqv_reader_big_endian_tpl. pending */
template<class T, class reader> class data_seqv_reader_big_endian_tpl
{
public:
   data_seqv_reader_big_endian_tpl(data_seqv_reader_base_tpl<T, reader>& i_reader) : reader_v(i_reader) {}
   // single data versions
   int8_t read_i8();
   uint8_t read_u8();
   int16_t read_i16();
   uint16_t read_u16();
   int32_t read_i32();
   uint32_t read_u32();
   int64_t read_i64();
   uint64_t read_u64();
   float read_f32();
   double read_f64();

   // seqv data versions. each returns the number of bytes read

   /** workhorse method. every other method ends up calling this */
   int read_bytes(std::byte* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   void read_i8(int8_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   void read_u8(uint8_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   //void read_i16(int16_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   //void read_u16(uint16_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   //void read_i32(int32_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   //void read_u32(uint32_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   //void read_i64(int64_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   //void read_u64(uint64_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   //void read_f32(float* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   //void read_f64(double* i_seqv, size_t i_elem_count, size_t i_offset = 0);

private:
   data_seqv_reader_base_tpl<T, reader>& reader_v;
};


/** base class for data sequence readers */
template<class T, class reader> class data_seqv_reader_base_tpl
{
public:
   data_seqv_reader_base_tpl(T i_seqv) : seqv_v(i_seqv) {}
   data_seqv_reader_base_tpl(const data_seqv_reader_base_tpl& i_seqv) : seqv_v(i_seqv.seqv_v) {}
   data_seqv_reader_base_tpl(data_seqv_reader_base_tpl&& i_seqv) noexcept { *this = std::move(i_seqv); }
   data_seqv_reader_base_tpl& operator=(const data_seqv_reader_base_tpl& i_seqv) = delete;
   data_seqv_reader_base_tpl& operator=(data_seqv_reader_base_tpl&& i_seqv) noexcept;
   /** returns the underlying data sequence */
   T& dsv();
   T& data_sequence() { return dsv(); }
   /** returns the underlying data sequence */
   const T& dsv() const;
   const T& data_sequence() const { return dsv(); }
   data_seqv_reader_big_endian_tpl<T, reader> be() { return data_seqv_reader_big_endian_tpl<T, reader>(*this); }
   data_seqv_reader_big_endian_tpl<T, reader> big_endian() { return be(); }
   bool is_end_of_seqv();
   // single data versions
   std::byte read_byte();
   char read_char();
   int8_t read_i8();
   uint8_t read_u8();
   int16_t read_i16();
   uint16_t read_u16();
   int32_t read_i32();
   uint32_t read_u32();
   int64_t read_i64();
   uint64_t read_u64();
   float read_f32();
   double read_f64();
   /** template method. can read any 'T0' that's a passive data structure(primitive, plain old data structure, etc) */
   template<class T0> T0 read();
   /** avoid using read_line(), as it's quite slow */
   std::string read_line();
   template<class T0> void read_pointer(T0*& i_seqv);

   // seqv data versions. each returns the number of bytes read

   /** workhorse method. every other method ends up calling this */
   int read_bytes(std::byte* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   /** reads a vector of bytes of length 'i_elem_count' */
   template<class byte_seqv = byte_vect> byte_seqv read_byte_vect(size_t i_elem_count);
   /** reads a sequence that was written with the write_sized_byte_vect() function.
   it's the same as read_byte_vect(), but the size is read from the sequence */
   template<class T0 = size_t, class byte_seqv = byte_vect> byte_seqv read_sized_byte_vect();
   /** reads a text(string) that was written with the write_sized_text() function */
   template<class T0 = size_t> std::string read_sized_text();
   /** template method. can read any 'T0' array that's a passive data structure(primitive, plain old data structure, etc) */
   template<class T0> int read(T0* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   int read_chars(char* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   std::string read_string(size_t i_elem_count);
   int read_i8(int8_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   int read_u8(uint8_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   int read_i16(int16_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   int read_u16(uint16_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   int read_i32(int32_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   int read_u32(uint32_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   int read_i64(int64_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   int read_u64(uint64_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   int read_f32(float* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   int read_f64(double* i_seqv, size_t i_elem_count, size_t i_offset = 0);

protected:
   T seqv_v;
};


/** ro memory sequence reader embedded instance version */
template<class byte_seqv> class data_seqv_ro_mem_reader_tpl : public data_seqv_reader_base_tpl<data_seqv_ro_mem_tpl<byte_seqv>, ref_adapter<data_seqv_tpl<byte_seqv>>>
{
public:
   /** constructs a ro memory data sequence with a pointer to data copied from i_seqv. i_seqv's size must NOT be modified while referenced by data_seqv_ro_mem_tpl */
   data_seqv_ro_mem_reader_tpl(data_seqv_ro_mem_tpl<byte_seqv> i_seqv) :
      data_seqv_reader_base_tpl<data_seqv_ro_mem_tpl<byte_seqv>, ref_adapter<data_seqv_tpl<byte_seqv>>>(i_seqv) {}
   /** constructs a ro memory data sequence with a pointer to data copied from i_seqv. i_seqv's size must NOT be modified while referenced by data_seqv_ro_mem_tpl */
   data_seqv_ro_mem_reader_tpl(const byte_seqv& i_seqv) :
      data_seqv_reader_base_tpl<data_seqv_ro_mem_tpl<byte_seqv>, ref_adapter<data_seqv_tpl<byte_seqv>>>(data_seqv_ro_mem_tpl<byte_seqv>(i_seqv.data(), i_seqv.size())) {}
   /** constructs a ro memory data sequence with a pointer to data copied from i_seqv. i_seqv's size must NOT be modified while referenced by data_seqv_ro_mem_tpl */
   data_seqv_ro_mem_reader_tpl(const std::byte* i_data, size_t i_size) :
      data_seqv_reader_base_tpl<data_seqv_ro_mem_tpl<byte_seqv>, ref_adapter<data_seqv_tpl<byte_seqv>>>(data_seqv_ro_mem_tpl<byte_seqv>(i_data, i_size)) {}
};

/** ro memory sequence reader reference version */
template<class byte_seqv> class data_seqv_ro_mem_reader_ref_tpl : public data_seqv_reader_base_tpl<data_seqv_ro_mem_tpl<byte_seqv>&, ref_adapter<data_seqv_tpl<byte_seqv>>>
{
public:
   /** constructs a ro memory data sequence with a pointer to data copied from i_seqv. i_seqv's size must NOT be modified while referenced by data_seqv_ro_mem_tpl */
   data_seqv_ro_mem_reader_ref_tpl(data_seqv_ro_mem_tpl<byte_seqv>& i_seqv) :
      data_seqv_reader_base_tpl<data_seqv_ro_mem_tpl<byte_seqv>&, ref_adapter<data_seqv_tpl<byte_seqv>>>(i_seqv) {}
};


/** memory sequence reader reference version */
template<class byte_seqv> class data_seqv_mem_reader_ref_tpl : public data_seqv_reader_base_tpl<data_seqv_rw_mem_tpl<byte_seqv>&, ref_adapter<data_seqv_tpl<byte_seqv>>>
{
public:
   /** constructs a rw memory data sequence with a reference to i_seqv */
   data_seqv_mem_reader_ref_tpl(data_seqv_rw_mem_tpl<byte_seqv>& i_seqv) :
      data_seqv_reader_base_tpl<data_seqv_rw_mem_tpl<byte_seqv>&, ref_adapter<data_seqv_tpl<byte_seqv>>>(i_seqv) {}
};


/** file sequence reader reference version */
template<class byte_seqv> class data_seqv_file_reader_ref_tpl : public data_seqv_reader_base_tpl<data_seqv_file_tpl<byte_seqv>&, ref_adapter<data_seqv_tpl<byte_seqv>>>
{
public:
   /** constructs a file data sequence with data copied from i_seqv */
   data_seqv_file_reader_ref_tpl(data_seqv_file_tpl<byte_seqv>& i_seqv) :
      data_seqv_reader_base_tpl<data_seqv_file_tpl<byte_seqv>&, ref_adapter<data_seqv_tpl<byte_seqv>>>(i_seqv) {}
};


/** sequence reader pointer version */
template<class byte_seqv> class data_seqv_reader_ptr_tpl : public data_seqv_reader_base_tpl<data_seqv_tpl<byte_seqv>*, ptr_adapter<data_seqv_tpl<byte_seqv>*>>
{
public:
   data_seqv_reader_ptr_tpl() : data_seqv_reader_base_tpl<data_seqv_tpl<byte_seqv>*, ptr_adapter<data_seqv_tpl<byte_seqv>*>>(nullptr) {}
   data_seqv_reader_ptr_tpl(data_seqv_tpl<byte_seqv>* i_seqv) : data_seqv_reader_base_tpl<data_seqv_tpl<byte_seqv>*, ptr_adapter<data_seqv_tpl<byte_seqv>*>>(i_seqv) {}
   void set_data_sequence(data_seqv_tpl<byte_seqv>* i_seqv) { this->seqv_v = i_seqv; }
};


/** sequence reader shared pointer version */
template<class byte_seqv> class data_seqv_reader_shr_tpl :
   public data_seqv_reader_base_tpl<std::shared_ptr<data_seqv_tpl<byte_seqv>>, ptr_adapter<std::shared_ptr<data_seqv_tpl<byte_seqv>>>>
{
public:
   data_seqv_reader_shr_tpl() : data_seqv_reader_base_tpl<std::shared_ptr<data_seqv_tpl<byte_seqv>>, ptr_adapter<std::shared_ptr<data_seqv_tpl<byte_seqv>>>>(nullptr) {}
   data_seqv_reader_shr_tpl(std::shared_ptr<data_seqv_tpl<byte_seqv>> i_seqv) :
      data_seqv_reader_base_tpl<std::shared_ptr<data_seqv_tpl<byte_seqv>>, ptr_adapter<std::shared_ptr<data_seqv_tpl<byte_seqv>>>>(i_seqv) {}
   void set_data_sequence(std::shared_ptr<data_seqv_tpl<byte_seqv>> i_seqv) { this->seqv_v = i_seqv; }
};


/** data_seqv_writer_big_endian_tpl. pending */
template<class T, class writer> class data_seqv_writer_big_endian_tpl
{
public:
   data_seqv_writer_big_endian_tpl(data_seqv_writer_base_tpl<T, writer>& i_writer) : writer_v(i_writer) {}

   // seqv data versions

   /** workhorse method. every other method ends up calling this */
   void write_bytes(const std::byte* i_seqv, size_t i_elem_count, size_t i_offset = 0);

private:
   data_seqv_writer_base_tpl<T, writer>& writer_v;
};


/** base class for data sequence writers */
template<class T, class writer> class data_seqv_writer_base_tpl
{
public:
   data_seqv_writer_base_tpl(T i_seqv) : seqv_v(i_seqv) {}
   data_seqv_writer_base_tpl(const data_seqv_writer_base_tpl& i_seqv) : seqv_v(i_seqv.seqv_v) {}
   data_seqv_writer_base_tpl(data_seqv_writer_base_tpl&& i_seqv) noexcept { *this = std::move(i_seqv); }
   data_seqv_writer_base_tpl& operator=(const data_seqv_writer_base_tpl& i_seqv) = delete;
   data_seqv_writer_base_tpl& operator=(data_seqv_writer_base_tpl&& i_seqv) noexcept;
   /** returns the underlying data sequence */
   T& dsv();
   T& data_sequence() { return dsv(); }
   /** returns the underlying data sequence */
   const T& dsv() const;
   const T& data_sequence() const { return dsv(); }
   data_seqv_writer_big_endian_tpl<T, writer> be() { return data_seqv_writer_big_endian_tpl<T, writer>(*this); }
   data_seqv_writer_big_endian_tpl<T, writer> big_endian() { return be(); }
   // single data versions
   void write_byte(std::byte d);
   void write_char(char d);
   void write_i8(int8_t d);
   void write_u8(uint8_t d);
   void write_i16(int16_t d);
   void write_u16(uint16_t d);
   void write_i32(int32_t d);
   void write_u32(uint32_t d);
   void write_i64(int64_t d);
   void write_u64(uint64_t d);
   void write_f32(float d);
   void write_f64(double d);
   /** template method. can write any 'T0' that's a passive data structure(primitive, plain old data structure, etc) */
   template<class T0> void write(const T0& i_data);
   void write_line(const std::string& i_text, bool i_new_line = true);
   template<class T0> void write_pointer(T0* const i_seqv);

   // seqv data versions

   /** workhorse method. every other method ends up calling this */
   void write_bytes(const std::byte* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   /** writes a vector of bytes */
   template<class byte_seqv = byte_vect> void write_byte_vect(const byte_seqv& i_seqv) { write_bytes(i_seqv.data(), i_seqv.size()); }
   /** same as write_byte_vect, but also writes in front the number of bytes in the sequence(an integer of type T0) */
   template<class T0 = size_t, class byte_seqv = byte_vect> void write_sized_byte_vect(const byte_seqv& i_seqv) { write_sized_byte_vect<T0>(i_seqv.data(), i_seqv.size()); }
   template<class T0 = size_t> void write_sized_byte_vect(const std::byte* i_seqv, T0 i_elem_count, T0 i_offset = 0);
   /** writes a text(string) and also writes in front the number of bytes in the sequence(an integer of type T0) */
   template<class T0 = size_t> void write_sized_text(const std::string& i_text);
   /** template method. can write any 'T0' array that's a passive data structure(primitive, plain old data structure, etc) */
   template<class T0> void write(const T0* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   void write_chars(const char* i_seqv, size_t i_elem_count = 0, size_t i_offset = 0);
   void write_string(const std::string& i_seqv);
   void write_i8(const int8_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   void write_u8(const uint8_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   void write_i16(const int16_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   void write_u16(const uint16_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   void write_i32(const int32_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   void write_u32(const uint32_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   void write_i64(const int64_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   void write_u64(const uint64_t* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   void write_f32(const float* i_seqv, size_t i_elem_count, size_t i_offset = 0);
   void write_f64(const double* i_seqv, size_t i_elem_count, size_t i_offset = 0);

protected:
   T seqv_v;
};


/** memory sequence writer embedded instance version */
template<class byte_seqv> class data_seqv_mem_writer_tpl : public data_seqv_writer_base_tpl<data_seqv_rw_mem_tpl<byte_seqv>, ref_adapter<data_seqv_tpl<byte_seqv>>>
{
public:
   /** constructs an empty rw memory data sequence */
   data_seqv_mem_writer_tpl() : data_seqv_writer_base_tpl<data_seqv_rw_mem_tpl<byte_seqv>, ref_adapter<data_seqv_tpl<byte_seqv>>>(data_seqv_rw_mem_tpl<byte_seqv>()) {}
   /** constructs a rw memory data sequence with its size the specified number of bytes */
   data_seqv_mem_writer_tpl(size_t i_elem_count) :
      data_seqv_writer_base_tpl<data_seqv_rw_mem_tpl<byte_seqv>, ref_adapter<data_seqv_tpl<byte_seqv>>>(data_seqv_rw_mem_tpl<byte_seqv>())
   {
      this->seqv_v.resize(i_elem_count);
   }
   /** constructs a rw memory data sequence with data copied from i_seqv */
   data_seqv_mem_writer_tpl(const data_seqv_mem_writer_tpl& i_seqv) :
      data_seqv_writer_base_tpl<data_seqv_rw_mem_tpl<byte_seqv>, ref_adapter<data_seqv_tpl<byte_seqv>>>(i_seqv) {}
   /** constructs a rw memory data sequence with data copied from i_seqv */
   data_seqv_mem_writer_tpl(const data_seqv_rw_mem_tpl<byte_seqv>& i_seqv) :
      data_seqv_writer_base_tpl<data_seqv_rw_mem_tpl<byte_seqv>, ref_adapter<data_seqv_tpl<byte_seqv>>>(i_seqv) {}
   /** constructs a rw memory data sequence with data copied from i_seqv */
   data_seqv_mem_writer_tpl(const byte_seqv& i_seqv) :
      data_seqv_writer_base_tpl<data_seqv_rw_mem_tpl<byte_seqv>, ref_adapter<data_seqv_tpl<byte_seqv>>>(data_seqv_rw_mem_tpl<byte_seqv>())
   {
      *this = i_seqv;
   }
   /** constructs a rw memory data sequence with data moved from i_seqv */
   data_seqv_mem_writer_tpl(data_seqv_mem_writer_tpl&& i_seqv) noexcept :
      data_seqv_writer_base_tpl<data_seqv_rw_mem_tpl<byte_seqv>, ref_adapter<data_seqv_tpl<byte_seqv>>>(std::move(i_seqv)) {}
   /** constructs a rw memory data sequence with data moved from i_seqv */
   data_seqv_mem_writer_tpl(data_seqv_rw_mem_tpl<byte_seqv>&& i_seqv) noexcept :
      data_seqv_writer_base_tpl<data_seqv_rw_mem_tpl<byte_seqv>, ref_adapter<data_seqv_tpl<byte_seqv>>>(data_seqv_rw_mem_tpl<byte_seqv>())
   {
      *this = std::move(i_seqv);
   }
   /** constructs a rw memory data sequence with data moved from i_seqv */
   data_seqv_mem_writer_tpl(byte_seqv&& i_seqv) noexcept :
      data_seqv_writer_base_tpl<data_seqv_rw_mem_tpl<byte_seqv>, ref_adapter<data_seqv_tpl<byte_seqv>>>(data_seqv_rw_mem_tpl<byte_seqv>())
   {
      *this = std::move(i_seqv);
   }
   data_seqv_mem_writer_tpl& operator=(const data_seqv_mem_writer_tpl& i_seqv);
   data_seqv_mem_writer_tpl& operator=(const data_seqv_rw_mem_tpl<byte_seqv>& i_seqv);
   data_seqv_mem_writer_tpl& operator=(const byte_seqv& i_seqv);
   data_seqv_mem_writer_tpl& operator=(data_seqv_mem_writer_tpl&& i_seqv) noexcept;
   data_seqv_mem_writer_tpl& operator=(data_seqv_rw_mem_tpl<byte_seqv>&& i_seqv) noexcept;
   data_seqv_mem_writer_tpl& operator=(byte_seqv&& i_seqv) noexcept;
};


/** memory sequence writer reference version */
template<class byte_seqv> class data_seqv_mem_writer_ref_tpl : public data_seqv_writer_base_tpl<data_seqv_rw_mem_tpl<byte_seqv>&, ref_adapter<data_seqv_tpl<byte_seqv>>>
{
public:
   /** constructs a rw memory data sequence with a reference to i_seqv */
   data_seqv_mem_writer_ref_tpl(data_seqv_rw_mem_tpl<byte_seqv>& i_seqv) :
      data_seqv_writer_base_tpl<data_seqv_rw_mem_tpl<byte_seqv>&, ref_adapter<data_seqv_tpl<byte_seqv>>>(i_seqv) {}
};


/** file sequence writer reference version */
template<class byte_seqv> class data_seqv_file_writer_ref_tpl : public data_seqv_writer_base_tpl<data_seqv_file_tpl<byte_seqv>&, ref_adapter<data_seqv_tpl<byte_seqv>>>
{
public:
   /** constructs a file data sequence with data copied from i_seqv */
   data_seqv_file_writer_ref_tpl(data_seqv_file_tpl<byte_seqv>& i_seqv) :
      data_seqv_writer_base_tpl<data_seqv_file_tpl<byte_seqv>&, ref_adapter<data_seqv_tpl<byte_seqv>>>(i_seqv) { assert(i_seqv.is_writable()); }
};


/** sequence writer pointer version */
template<class byte_seqv> class data_seqv_writer_ptr_tpl : public data_seqv_writer_base_tpl<data_seqv_tpl<byte_seqv>*, ptr_adapter<data_seqv_tpl<byte_seqv>*>>
{
public:
   data_seqv_writer_ptr_tpl() : data_seqv_writer_base_tpl(nullptr) {}
   data_seqv_writer_ptr_tpl(data_seqv_tpl<byte_seqv>* i_seqv) :
      data_seqv_writer_base_tpl<data_seqv_tpl<byte_seqv>*, ptr_adapter<data_seqv_tpl<byte_seqv>*>>(i_seqv) { assert(i_seqv->is_writable()); }
   void set_data_sequence(data_seqv_tpl<byte_seqv>* i_seqv) { assert(i_seqv->is_writable()); this->seqv_v = i_seqv; }
};


/** sequence writer shared pointer version */
template<class byte_seqv> class data_seqv_writer_shr_tpl :
   public data_seqv_writer_base_tpl<std::shared_ptr<data_seqv_tpl<byte_seqv>>, ptr_adapter<std::shared_ptr<data_seqv_tpl<byte_seqv>>>>
{
public:
   data_seqv_writer_shr_tpl() : data_seqv_writer_base_tpl<std::shared_ptr<data_seqv_tpl<byte_seqv>>, ptr_adapter<std::shared_ptr<data_seqv_tpl<byte_seqv>>>>(nullptr) {}
   data_seqv_writer_shr_tpl(std::shared_ptr<data_seqv_tpl<byte_seqv>> i_seqv) :
      data_seqv_writer_base_tpl<std::shared_ptr<data_seqv_tpl<byte_seqv>>, ptr_adapter<std::shared_ptr<data_seqv_tpl<byte_seqv>>>>(i_seqv) { assert(i_seqv->is_writable()); }
   void set_data_sequence(std::shared_ptr<data_seqv_tpl<byte_seqv>> i_seqv) { assert(i_seqv->is_writable()); this->seqv_v = i_seqv; }
};


/** implements read/write operations on a memory data sequence */
template<class byte_seqv> class data_seqv_rw_mem_ops_tpl : public data_seqv_rw_mem_tpl<byte_seqv>
{
public:
   /** constructs an empty rw memory data sequence */
   data_seqv_rw_mem_ops_tpl() : r(*this), w(*this) {}
   /** constructs a rw memory data sequence with its size the specified number of bytes */
   data_seqv_rw_mem_ops_tpl(size_t i_elem_count) : data_seqv_rw_mem_tpl<byte_seqv>(i_elem_count), r(*this), w(*this) {}
   /** constructs a rw memory data sequence with data copied from i_seqv */
   data_seqv_rw_mem_ops_tpl(const data_seqv_rw_mem_tpl<byte_seqv>& i_seqv) : data_seqv_rw_mem_tpl<byte_seqv>(i_seqv), r(*this), w(*this) {}
   /** constructs a rw memory data sequence with data moved from i_seqv */
   data_seqv_rw_mem_ops_tpl(data_seqv_rw_mem_tpl<byte_seqv>&& i_seqv) noexcept : data_seqv_rw_mem_tpl<byte_seqv>(std::move(i_seqv)), r(*this), w(*this) {}
   /** constructs a rw memory data sequence with data moved from i_seqv */
   data_seqv_rw_mem_ops_tpl(byte_seqv&& i_seqv) noexcept : r(*this), w(*this) { data_seqv_rw_mem_tpl<byte_seqv>::operator=(std::move(i_seqv)); }

   data_seqv_mem_reader_ref_tpl<byte_seqv> r;
   data_seqv_mem_writer_ref_tpl<byte_seqv> w;
};


/** implements read/write operations on a file data sequence */
template<class byte_seqv> class data_seqv_rw_file_ops_tpl : public data_seqv_file_tpl<byte_seqv>
{
public:
   data_seqv_rw_file_ops_tpl(const data_seqv_std_file_wrapper& i_file) : data_seqv_file_tpl<byte_seqv>(i_file), r(*this), w(*this) { assert(i_file.is_writable()); }

   data_seqv_file_reader_ref_tpl<byte_seqv> r;
   data_seqv_file_writer_ref_tpl<byte_seqv> w;
};


template<class byte_seqv> class data_seqv_rw_ops_ptr_tpl
{
public:
   data_seqv_rw_ops_ptr_tpl() {}
   data_seqv_rw_ops_ptr_tpl(data_seqv_tpl<byte_seqv>* i_seqv) : seqv_v(i_seqv), r(i_seqv), w(i_seqv) {}

   void set_data_sequence(data_seqv_tpl<byte_seqv>* i_seqv)
   {
      assert(i_seqv->is_writable());
      seqv_v = i_seqv;
      r.set_data_sequence(i_seqv);
      w.set_data_sequence(i_seqv);
   }

   data_seqv_tpl<byte_seqv>* dsv() const { return seqv_v; }
   data_seqv_tpl<byte_seqv>* data_sequence() const { return dsv(); }

   data_seqv_reader_ptr_tpl<byte_seqv> r;
   data_seqv_writer_ptr_tpl<byte_seqv> w;

protected:
   /** reference only! */
   data_seqv_tpl<byte_seqv>* seqv_v = nullptr;
};


template<class byte_seqv> class data_seqv_rw_ops_shr_tpl
{
public:
   data_seqv_rw_ops_shr_tpl() {}
   data_seqv_rw_ops_shr_tpl(std::shared_ptr<data_seqv_tpl<byte_seqv>> i_seqv) : seqv_v(i_seqv), r(i_seqv), w(i_seqv) {}

   void set_data_sequence(std::shared_ptr<data_seqv_tpl<byte_seqv>> i_seqv)
   {
      assert(i_seqv->is_writable());
      seqv_v = i_seqv;
      r.set_data_sequence(i_seqv);
      w.set_data_sequence(i_seqv);
   }

   std::shared_ptr<data_seqv_tpl<byte_seqv>> dsv() const { return seqv_v; }
   std::shared_ptr<data_seqv_tpl<byte_seqv>> data_sequence() const { return dsv(); }

   data_seqv_reader_shr_tpl<byte_seqv> r;
   data_seqv_writer_shr_tpl<byte_seqv> w;

protected:
   std::shared_ptr<data_seqv_tpl<byte_seqv>> seqv_v;
};


/** data_seqv_exception */
class data_seqv_exception
#ifdef __cpp_exceptions
   : public std::exception
#if !defined(mws_throw)
#define mws_throw throw
#endif
#else
#if !defined(mws_throw)
#define mws_throw
#endif
#endif
{
public:
   data_seqv_exception();
   data_seqv_exception(const std::string& i_msg);
   data_seqv_exception(const char* i_msg);
   virtual ~data_seqv_exception();
   /** returns a C-style character string describing the general cause of the current error */
   virtual const char* what() const noexcept;
   static void throw_ex(const char* i_msg = "n/a");

private:
   void set_msg(const char* i_msg);

   std::string msg;
};










// implementation // implementation // implementation // implementation // implementation










// data_seqv_tpl
template<class byte_seqv> data_seqv_tpl<byte_seqv>::data_seqv_tpl() { data_seqv_tpl::reset(); }

template<class byte_seqv> data_seqv_tpl<byte_seqv>& data_seqv_tpl<byte_seqv>::operator=(const data_seqv_tpl<byte_seqv>& i_seqv)
{
   if (this != &i_seqv)
   {
      read_position_v = i_seqv.read_position_v;
      write_position_v = i_seqv.write_position_v;
#ifdef log_bytes_rw
      total_bytes_read_v = i_seqv.total_bytes_read_v;
      total_bytes_written_v = i_seqv.total_bytes_written_v;
#endif
   }

   return *this;
}

template<class byte_seqv> data_seqv_tpl<byte_seqv>& data_seqv_tpl<byte_seqv>::operator=(data_seqv_tpl<byte_seqv>&& i_seqv) noexcept
{
   if (this != &i_seqv)
   {
      read_position_v = i_seqv.read_position_v;
      write_position_v = i_seqv.write_position_v;
#ifdef log_bytes_rw
      total_bytes_read_v = i_seqv.total_bytes_read_v;
      total_bytes_written_v = i_seqv.total_bytes_written_v;
#endif
      i_seqv.data_seqv_tpl::reset();
   }

   return *this;
}

template<class byte_seqv> bool data_seqv_tpl<byte_seqv>::is_end_of_seqv() { return read_position() >= size(); }
template<class byte_seqv> void data_seqv_tpl<byte_seqv>::close() {}
template<class byte_seqv> uint64_t data_seqv_tpl<byte_seqv>::read_position() const { return read_position_v; }
template<class byte_seqv> uint64_t data_seqv_tpl<byte_seqv>::write_position() const { return write_position_v; }

template<class byte_seqv> uint64_t data_seqv_tpl<byte_seqv>::total_bytes_read() const
{
#ifdef log_bytes_rw
   return total_bytes_read_v;
#else
   return 0;
#endif
}

template<class byte_seqv> uint64_t data_seqv_tpl<byte_seqv>::total_bytes_written() const
{
#ifdef log_bytes_rw
   return total_bytes_written_v;
#else
   return 0;
#endif
}

template<class byte_seqv> void data_seqv_tpl<byte_seqv>::rewind()
{
   read_position_v = write_position_v = 0;
#ifdef log_bytes_rw
   total_bytes_read_v = total_bytes_written_v = 0;
#endif
}

template<class byte_seqv> void data_seqv_tpl<byte_seqv>::reset() { rewind(); }
template<class byte_seqv> void data_seqv_tpl<byte_seqv>::set_io_position(uint64_t i_position) { read_position_v = write_position_v = i_position; }
template<class byte_seqv> void data_seqv_tpl<byte_seqv>::set_read_position(uint64_t i_position) { read_position_v = i_position; }
template<class byte_seqv> void data_seqv_tpl<byte_seqv>::set_write_position(uint64_t i_position) { write_position_v = i_position; }

template<class byte_seqv> int data_seqv_tpl<byte_seqv>::read_bytes(std::byte* i_seqv, size_t i_elem_count, size_t i_offset)
{
   int bytes_read = read_bytes_impl(i_seqv, i_elem_count, i_offset);

   read_position_v += bytes_read;
#ifdef log_bytes_rw
   total_bytes_read_v += bytes_read;
#endif

   return bytes_read;
}

template<class byte_seqv> int data_seqv_tpl<byte_seqv>::write_bytes(const std::byte* i_seqv, size_t i_elem_count, size_t i_offset)
{
   int bytes_written = write_bytes_impl(i_seqv, i_elem_count, i_offset);

   write_position_v += bytes_written;
#ifdef log_bytes_rw
   total_bytes_written_v += bytes_written;
#endif

   return bytes_written;
}


// data_seqv_ro_mem_tpl
template<class byte_seqv> data_seqv_ro_mem_tpl<byte_seqv>::data_seqv_ro_mem_tpl(const byte_seqv& i_seqv) : seqv_v(i_seqv.data()), size_v(i_seqv.size()) {}
template<class byte_seqv> data_seqv_ro_mem_tpl<byte_seqv>::data_seqv_ro_mem_tpl(const std::byte* i_seqv, uint64_t i_elem_count) : seqv_v(i_seqv), size_v(i_elem_count) {}
template<class byte_seqv> const std::byte* data_seqv_ro_mem_tpl<byte_seqv>::seqv_as_array() const { return seqv_v; }
template<class byte_seqv> uint64_t data_seqv_ro_mem_tpl<byte_seqv>::size() const { return size_v; }
template<class byte_seqv> void data_seqv_ro_mem_tpl<byte_seqv>::rewind() { data_seqv_tpl<byte_seqv>::rewind(); }
template<class byte_seqv> void data_seqv_ro_mem_tpl<byte_seqv>::reset() { data_seqv_tpl<byte_seqv>::reset(); }
template<class byte_seqv> void data_seqv_ro_mem_tpl<byte_seqv>::set_io_position(uint64_t i_position) { set_read_position(i_position); }

template<class byte_seqv> byte_seqv data_seqv_ro_mem_tpl<byte_seqv>::seqv_as_vector()
{
   byte_seqv sq;
   size_t sz = static_cast<size_t>(size());

   if (sz > 0)
   {
      sq.assign(seqv_v, seqv_v + sz);
   }

   return sq;
}

template<class byte_seqv> void data_seqv_ro_mem_tpl<byte_seqv>::set_read_position(uint64_t i_pos)
{
   if (i_pos > size())
   {
      data_seqv_exception::throw_ex();
   }
   else
   {
      this->read_position_v = i_pos;
   }
}

template<class byte_seqv> void data_seqv_ro_mem_tpl<byte_seqv>::set_write_position(uint64_t) { data_seqv_exception::throw_ex(); }

template<class byte_seqv> int data_seqv_ro_mem_tpl<byte_seqv>::read_bytes_impl(std::byte* i_seqv, size_t i_elem_count, size_t i_offset)
{
   int bytes_to_read = 0;

   if (this->read_position() < size())
   {
      size_t read_pos = static_cast<size_t>(this->read_position());
      std::byte* dst = &i_seqv[i_offset];
      const std::byte* src = &seqv_v[read_pos];

      bytes_to_read = static_cast<int>(std::min(i_elem_count, static_cast<size_t>(size() - this->read_position())));
      std::memcpy(dst, src, bytes_to_read);
   }

   return bytes_to_read;
}

template<class byte_seqv> int data_seqv_ro_mem_tpl<byte_seqv>::write_bytes_impl(const std::byte*, size_t, size_t) { data_seqv_exception::throw_ex(); return -1; }


// data_seqv_rw_mem_tpl
template<class byte_seqv> data_seqv_rw_mem_tpl<byte_seqv>::data_seqv_rw_mem_tpl(const std::byte* i_seqv, size_t i_elem_count)
{
   seqv_v.assign(i_seqv, i_seqv + i_elem_count);
   set_write_position(size());
}

template<class byte_seqv> data_seqv_rw_mem_tpl<byte_seqv>& data_seqv_rw_mem_tpl<byte_seqv>::operator=(const data_seqv_rw_mem_tpl<byte_seqv>& i_seqv)
{
   if (this != &i_seqv)
   {
      data_seqv_tpl<byte_seqv>::operator=(i_seqv);
      seqv_v = i_seqv.seqv_v;
   }

   return *this;
}

template<class byte_seqv> data_seqv_rw_mem_tpl<byte_seqv>& data_seqv_rw_mem_tpl<byte_seqv>::operator=(const byte_seqv& i_seqv)
{
   if (&seqv_v != &i_seqv)
   {
      seqv_v = i_seqv;
      set_write_position(size());
   }

   return *this;
}

template<class byte_seqv> data_seqv_rw_mem_tpl<byte_seqv>& data_seqv_rw_mem_tpl<byte_seqv>::operator=(data_seqv_rw_mem_tpl<byte_seqv>&& i_seqv) noexcept
{
   if (this != &i_seqv)
   {
      data_seqv_tpl<byte_seqv>::operator=(std::move(i_seqv));
      seqv_v = std::move(i_seqv.seqv_v);
   }

   return *this;
}

template<class byte_seqv> data_seqv_rw_mem_tpl<byte_seqv>& data_seqv_rw_mem_tpl<byte_seqv>::operator=(byte_seqv&& i_seqv) noexcept
{
   if (&seqv_v != &i_seqv)
   {
      seqv_v = std::move(i_seqv);
      set_write_position(size());
   }

   return *this;
}

template<class byte_seqv> const std::byte* data_seqv_rw_mem_tpl<byte_seqv>::seqv_as_array() const { return seqv_v.data(); }
template<class byte_seqv> byte_seqv data_seqv_rw_mem_tpl<byte_seqv>::seqv_as_vector() { return seqv_v; }
template<class byte_seqv> uint64_t data_seqv_rw_mem_tpl<byte_seqv>::size() const { return seqv_v.size(); }
template<class byte_seqv> void data_seqv_rw_mem_tpl<byte_seqv>::rewind() { data_seqv_tpl<byte_seqv>::rewind(); }
template<class byte_seqv> void data_seqv_rw_mem_tpl<byte_seqv>::reset() { data_seqv_tpl<byte_seqv>::reset(); seqv_v.clear(); }
template<class byte_seqv> void data_seqv_rw_mem_tpl<byte_seqv>::set_io_position(uint64_t i_position) { set_read_position(i_position); set_write_position(i_position); }
template<class byte_seqv> void data_seqv_rw_mem_tpl<byte_seqv>::set_read_position(uint64_t i_pos)
{
   if (i_pos > size())
   {
      data_seqv_exception::throw_ex();
   }
   else
   {
      this->read_position_v = i_pos;
   }
}

template<class byte_seqv> void data_seqv_rw_mem_tpl<byte_seqv>::set_write_position(uint64_t i_pos)
{
   if (i_pos > size())
   {
      data_seqv_exception::throw_ex();
   }
   else
   {
      this->write_position_v = i_pos;
   }
}

template<class byte_seqv> void data_seqv_rw_mem_tpl<byte_seqv>::resize(size_t i_elem_count)
{
   seqv_v.resize(i_elem_count);

   if (this->read_position_v >= i_elem_count)
   {
      set_read_position(i_elem_count);
   }

   if (this->write_position_v >= i_elem_count)
   {
      set_write_position(i_elem_count);
   }
}

template<class byte_seqv> void data_seqv_rw_mem_tpl<byte_seqv>::move_into(byte_seqv& i_seqv)
{
   i_seqv.swap(seqv_v);
   reset();
}

template<class byte_seqv> int data_seqv_rw_mem_tpl<byte_seqv>::read_bytes_impl(std::byte* i_seqv, size_t i_elem_count, size_t i_offset)
{
   int bytes_to_read = 0;

   if (i_elem_count > 0 && (this->read_position() < size()))
   {
      size_t read_pos = static_cast<size_t>(this->read_position());
      std::byte* dst = &i_seqv[i_offset];
      const std::byte* src = &seqv_v[read_pos];

      bytes_to_read = static_cast<int>(std::min(i_elem_count, static_cast<size_t>(size() - this->read_position())));
      std::memcpy(dst, src, bytes_to_read);
   }

   return bytes_to_read;
}

template<class byte_seqv> int data_seqv_rw_mem_tpl<byte_seqv>::write_bytes_impl(const std::byte* i_seqv, size_t i_elem_count, size_t i_offset)
{
   if (i_elem_count > 0)
   {
      if (size() - this->write_position() < i_elem_count)
      {
         seqv_v.resize(static_cast<size_t>(size()) + i_elem_count);
      }

      size_t write_pos = static_cast<size_t>(this->write_position());
      std::byte* dst = &seqv_v[write_pos];
      const std::byte* src = &i_seqv[i_offset];

      std::memcpy(dst, src, i_elem_count);
   }

   return i_elem_count;
}


// data_seqv_file_base_tpl
// data_seqv_std_file_wrapper
inline data_seqv_std_file_wrapper::data_seqv_std_file_wrapper(const std::string& i_file_path, const std::string& i_open_mode)
{
   const size_t np = std::string::npos;
   is_writable_v = (i_open_mode.find('a') != np) || (i_open_mode.find('w') != np) || (i_open_mode.find('+') != np);
#pragma warning(suppress : 4996)
   std::FILE* f = fopen(i_file_path.c_str(), i_open_mode.c_str());

   if (f)
   {
      file_v = std::shared_ptr<std::FILE>(f, std::fclose);
   }
}

inline data_seqv_std_file_wrapper::data_seqv_std_file_wrapper() : is_writable_v(false) {}
inline data_seqv_std_file_wrapper::data_seqv_std_file_wrapper(std::shared_ptr<std::FILE> i_file, bool i_is_writable) : file_v(i_file), is_writable_v(i_is_writable) {}
inline bool data_seqv_std_file_wrapper::is_open() const { return file_v != nullptr; }
inline bool data_seqv_std_file_wrapper::is_writable() const { return is_writable_v; }
inline uint64_t data_seqv_std_file_wrapper::length() const { std::FILE* f = file_ptr(); fseek(f, 0L, SEEK_END); long size = ftell(f); rewind(f); return size; }
inline void data_seqv_std_file_wrapper::close() { fclose(file_ptr()); }
inline void data_seqv_std_file_wrapper::set_io_position(uint64_t i_position) { fseek(file_ptr(), static_cast<long>(i_position), 0); }
inline std::FILE* data_seqv_std_file_wrapper::file_ptr() const { return file_v.get(); }

inline int data_seqv_std_file_wrapper::read_bytes(std::byte* i_seqv, size_t i_elem_count, size_t i_offset)
{
   return fread(i_seqv + i_offset, 1, i_elem_count, file_ptr());
}

inline int data_seqv_std_file_wrapper::write_bytes(const std::byte* i_seqv, size_t i_elem_count, size_t i_offset)
{
   return fwrite(i_seqv + i_offset, 1, i_elem_count, file_ptr());
}


// data_seqv_file_base_tpl
template<class T, class io, class byte_seqv> bool data_seqv_file_base_tpl<T, io, byte_seqv>::is_end_of_seqv()
{
   uint64_t file_size = size();

   if (this->write_position() > file_size)
   {
      return this->read_position() >= this->write_position();
   }

   return this->read_position() >= file_size;
}

template<class T, class io, class byte_seqv>  byte_seqv data_seqv_file_base_tpl<T, io, byte_seqv>::seqv_as_vector()
{
   uint64_t read_pos = this->read_position();
   uint64_t file_size = size();
   byte_seqv bytes(static_cast<size_t>(file_size));

   set_read_position(0);
   read_bytes_impl(bytes.data(), bytes.size(), 0);
   set_read_position(read_pos);

   return bytes;
}

template<class T, class io, class byte_seqv> void data_seqv_file_base_tpl<T, io, byte_seqv>::close() { io()(file_v)->close(); }
template<class T, class io, class byte_seqv> uint64_t data_seqv_file_base_tpl<T, io, byte_seqv>::size() const { return io()(file_v)->length(); }
template<class T, class io, class byte_seqv> void data_seqv_file_base_tpl<T, io, byte_seqv>::rewind() { data_seqv_tpl<byte_seqv>::rewind(); set_io_position(0); }
template<class T, class io, class byte_seqv> void data_seqv_file_base_tpl<T, io, byte_seqv>::reset() { data_seqv_tpl<byte_seqv>::reset(); set_io_position(0); }
template<class T, class io, class byte_seqv> const T& data_seqv_file_base_tpl<T, io, byte_seqv>::file() const { return file_v; }

template<class T, class io, class byte_seqv> void data_seqv_file_base_tpl<T, io, byte_seqv>::set_io_position(uint64_t i_pos)
{
   last_file_pos = this->read_position_v = this->write_position_v = i_pos;
   io()(file_v)->set_io_position(i_pos);
}

template<class T, class io, class byte_seqv> void data_seqv_file_base_tpl<T, io, byte_seqv>::set_read_position(uint64_t i_pos)
{
   last_file_pos = this->read_position_v = i_pos;
   io()(file_v)->set_io_position(i_pos);
}

template<class T, class io, class byte_seqv> void data_seqv_file_base_tpl<T, io, byte_seqv>::set_write_position(uint64_t i_pos)
{
   last_file_pos = this->write_position_v = i_pos;
   io()(file_v)->set_io_position(i_pos);
}

template<class T, class io, class byte_seqv> int data_seqv_file_base_tpl<T, io, byte_seqv>::read_bytes_impl(std::byte* i_seqv, size_t i_elem_count, size_t i_offset)
{
   int bytes_read = 0;

   if (this->read_position() < size())
   {
      if (this->read_position() != last_file_pos)
      {
         set_read_position(this->read_position());
      }

      bytes_read = io()(file_v)->read_bytes(i_seqv, i_elem_count, i_offset);
      last_file_pos = this->read_position();
   }

   if (bytes_read < 0 || static_cast<size_t>(bytes_read) != i_elem_count)
   {
      data_seqv_exception::throw_ex("reached end of file");
   }

   return bytes_read;
}

template<class T, class io, class byte_seqv> int data_seqv_file_base_tpl<T, io, byte_seqv>::
write_bytes_impl(const std::byte* i_seqv, size_t i_elem_count, size_t i_offset)
{
   if (this->write_position() != last_file_pos)
   {
      set_write_position(this->write_position());
   }

   int bytes_written = io()(file_v)->write_bytes(i_seqv, i_elem_count, i_offset);
   last_file_pos = this->write_position();

   return bytes_written;
}


// data_seqv_reader_big_endian_tpl
template<class T, class reader> int8_t data_seqv_reader_big_endian_tpl<T, reader>::read_i8()
{
   int8_t sq;
   read_bytes(byte_cast(&sq), 1, 0);

   return sq;
}

template<class T, class reader> uint8_t data_seqv_reader_big_endian_tpl<T, reader>::read_u8()
{
   return (uint8_t)read_i8();
}

template<class T, class reader> int16_t data_seqv_reader_big_endian_tpl<T, reader>::read_i16()
{
   int8_t sq[2];
   read_bytes(byte_cast(sq), 2, 0);
   int16_t r = ((sq[1] & 0xff) | (sq[0] & 0xff) << 8);

   return r;
}

template<class T, class reader> uint16_t data_seqv_reader_big_endian_tpl<T, reader>::read_u16()
{
   return (uint16_t)read_i16();
}

template<class T, class reader> int32_t data_seqv_reader_big_endian_tpl<T, reader>::read_i32()
{
   int8_t sq[4];
   read_bytes(byte_cast(sq), 4, 0);
   int32_t r = (sq[3] & 0xff) | ((sq[2] & 0xff) << 8) | ((sq[1] & 0xff) << 16) | ((sq[0] & 0xff) << 24);

   return r;
}

template<class T, class reader> uint32_t data_seqv_reader_big_endian_tpl<T, reader>::read_u32()
{
   return (uint32_t)read_i32();
}

template<class T, class reader> int64_t data_seqv_reader_big_endian_tpl<T, reader>::read_i64()
{
   int8_t sq[8];
   read_bytes(byte_cast(sq), 8, 0);
   int64_t r = ((int64_t)(sq[7] & 0xff) | ((int64_t)(sq[6] & 0xff) << 8) | ((int64_t)(sq[5] & 0xff) << 16) | ((int64_t)(sq[4] & 0xff) << 24) |
      ((int64_t)(sq[3] & 0xff) << 32) | ((int64_t)(sq[2] & 0xff) << 40) | ((int64_t)(sq[1] & 0xff) << 48) | ((int64_t)(sq[0] & 0xff) << 56));

   return r;
}

template<class T, class reader> uint64_t data_seqv_reader_big_endian_tpl<T, reader>::read_u64()
{
   return (uint64_t)read_i64();
}

template<class T, class reader> float data_seqv_reader_big_endian_tpl<T, reader>::read_f32()
{
   int32_t r = read_i32();

   return *(float*)&r;
}

template<class T, class reader> double data_seqv_reader_big_endian_tpl<T, reader>::read_f64()
{
   int64_t r = read_i64();

   return *(double*)&r;
}

template<class T, class reader> int data_seqv_reader_big_endian_tpl<T, reader>::read_bytes(std::byte* i_seqv, size_t i_elem_count, size_t i_offset)
{
   return reader_v.read_bytes(i_seqv, i_elem_count, i_offset);
}

template<class T, class reader> void data_seqv_reader_big_endian_tpl<T, reader>::read_i8(int8_t* i_seqv, size_t i_elem_count, size_t i_offset)
{
   read_bytes(byte_cast(i_seqv), i_elem_count, i_offset);
}

template<class T, class reader> void data_seqv_reader_big_endian_tpl<T, reader>::read_u8(uint8_t* i_seqv, size_t i_elem_count, size_t i_offset)
{
   read_bytes(byte_cast(i_seqv), i_elem_count, i_offset);
}


// data_seqv_reader_base_tpl
template<class T, class reader> data_seqv_reader_base_tpl<T, reader>& data_seqv_reader_base_tpl<T, reader>::operator=(data_seqv_reader_base_tpl<T, reader>&& i_seqv) noexcept
{
   if (this != &i_seqv)
   {
      seqv_v = std::move(i_seqv.seqv_v);
   }

   return *this;
}

template<class T, class reader> T& data_seqv_reader_base_tpl<T, reader>::dsv() { return seqv_v; }
template<class T, class reader> const T& data_seqv_reader_base_tpl<T, reader>::dsv() const { return seqv_v; }
template<class T, class reader> bool data_seqv_reader_base_tpl<T, reader>::is_end_of_seqv() { return reader()(seqv_v)->is_end_of_seqv(); }
template<class T, class reader> std::byte data_seqv_reader_base_tpl<T, reader>::read_byte() { return read<std::byte>(); }
template<class T, class reader> char data_seqv_reader_base_tpl<T, reader>::read_char() { return read<char>(); }
template<class T, class reader> int8_t data_seqv_reader_base_tpl<T, reader>::read_i8() { return read<int8_t>(); }
template<class T, class reader> uint8_t data_seqv_reader_base_tpl<T, reader>::read_u8() { return read<uint8_t>(); }
template<class T, class reader> int16_t data_seqv_reader_base_tpl<T, reader>::read_i16() { return read<int16_t>(); }
template<class T, class reader> uint16_t data_seqv_reader_base_tpl<T, reader>::read_u16() { return read<uint16_t>(); }
template<class T, class reader> int32_t data_seqv_reader_base_tpl<T, reader>::read_i32() { return read<int32_t>(); }
template<class T, class reader> uint32_t data_seqv_reader_base_tpl<T, reader>::read_u32() { return read<uint32_t>(); }
template<class T, class reader> int64_t data_seqv_reader_base_tpl<T, reader>::read_i64() { return read<int64_t>(); }
template<class T, class reader> uint64_t data_seqv_reader_base_tpl<T, reader>::read_u64() { return read<uint64_t>(); }
template<class T, class reader> float data_seqv_reader_base_tpl<T, reader>::read_f32() { return read<float>(); }
template<class T, class reader> double data_seqv_reader_base_tpl<T, reader>::read_f64() { return read<double>(); }

template<class T, class reader> template<class T0> T0 data_seqv_reader_base_tpl<T, reader>::read()
{
   union type_alias { T0 t; std::byte sq[sizeof(T0)]; } ta;
   read_bytes(ta.sq, sizeof(T0), 0);
   return ta.t;
}

template<class T, class reader> std::string data_seqv_reader_base_tpl<T, reader>::read_line()
{
   std::string text;
   std::vector<char> line;
   int8_t c = 0;
   line.reserve(256);

   while (true)
   {
      int bytes_read = read_i8(&c, 1, 0);

      if (bytes_read <= 0 || (c == '\n') || (c == '\r'))
      {
         break;
      }

      line.push_back(c);
   }

   if (!line.empty())
   {
      text = std::string(line.data(), line.size());
   }

   return text;
}

template<class T, class reader> template<class T0> void data_seqv_reader_base_tpl<T, reader>::read_pointer(T0*& i_seqv)
{
   read_bytes(byte_cast(&i_seqv), sizeof(uintptr_t), 0);
}

template<class T, class reader> int data_seqv_reader_base_tpl<T, reader>::read_bytes(std::byte* i_seqv, size_t i_elem_count, size_t i_offset)
{
   return reader()(seqv_v)->read_bytes(i_seqv, i_elem_count, i_offset);
}

template<class T, class reader> template<class byte_seqv> byte_seqv data_seqv_reader_base_tpl<T, reader>::read_byte_vect(size_t i_elem_count)
{
   byte_seqv vect(i_elem_count);
   int bytes_read = read_bytes(vect.data(), i_elem_count, 0);
   assert(static_cast<size_t>(bytes_read) == i_elem_count);
   return vect;
}

template<class T, class reader> template<class T0, class byte_seqv> byte_seqv data_seqv_reader_base_tpl<T, reader>::read_sized_byte_vect()
{
   T0 size = read<T0>();
   return read_byte_vect<byte_seqv>(size);
}

template<class T, class reader> template<class T0> std::string data_seqv_reader_base_tpl<T, reader>::read_sized_text()
{
   static_assert(sizeof(std::byte) == sizeof(char));
   T0 elem_count = read<T0>();
   std::string text(elem_count, 0);
   read_bytes(byte_cast(text.data()), elem_count);

   return text;
}

template<class T, class reader> template<class T0> int data_seqv_reader_base_tpl<T, reader>::read(T0* i_seqv, size_t i_elem_count, size_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count * sizeof(T0), i_offset);
}

template<class T, class reader> int data_seqv_reader_base_tpl<T, reader>::read_chars(char* i_seqv, size_t i_elem_count, size_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count, i_offset);
}

template<class T, class reader> std::string data_seqv_reader_base_tpl<T, reader>::read_string(size_t i_elem_count)
{
   std::string text(i_elem_count, 0);
   [[maybe_unused]] int bytes_read = read_bytes(byte_cast(text.data()), i_elem_count);
   assert(static_cast<size_t>(bytes_read) == i_elem_count);
   return text;
}

template<class T, class reader> int data_seqv_reader_base_tpl<T, reader>::read_i8(int8_t* i_seqv, size_t i_elem_count, size_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count, i_offset);
}

template<class T, class reader> int data_seqv_reader_base_tpl<T, reader>::read_u8(uint8_t* i_seqv, size_t i_elem_count, size_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count, i_offset);
}

template<class T, class reader> int data_seqv_reader_base_tpl<T, reader>::read_i16(int16_t* i_seqv, size_t i_elem_count, size_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count * sizeof(int16_t), i_offset * sizeof(int16_t));
}

template<class T, class reader> int data_seqv_reader_base_tpl<T, reader>::read_u16(uint16_t* i_seqv, size_t i_elem_count, size_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count * sizeof(uint16_t), i_offset * sizeof(uint16_t));
}

template<class T, class reader> int data_seqv_reader_base_tpl<T, reader>::read_i32(int32_t* i_seqv, size_t i_elem_count, size_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count * sizeof(int32_t), i_offset * sizeof(int32_t));
}

template<class T, class reader> int data_seqv_reader_base_tpl<T, reader>::read_u32(uint32_t* i_seqv, size_t i_elem_count, size_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count * sizeof(uint32_t), i_offset * sizeof(uint32_t));
}

template<class T, class reader> int data_seqv_reader_base_tpl<T, reader>::read_i64(int64_t* i_seqv, size_t i_elem_count, size_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count * sizeof(int64_t), i_offset * sizeof(int64_t));
}

template<class T, class reader> int data_seqv_reader_base_tpl<T, reader>::read_u64(uint64_t* i_seqv, size_t i_elem_count, size_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count * sizeof(uint64_t), i_offset * sizeof(uint64_t));
}

template<class T, class reader> int data_seqv_reader_base_tpl<T, reader>::read_f32(float* i_seqv, size_t i_elem_count, size_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count * sizeof(float), i_offset * sizeof(float));
}

template<class T, class reader> int data_seqv_reader_base_tpl<T, reader>::read_f64(double* i_seqv, size_t i_elem_count, size_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count * sizeof(double), i_offset * sizeof(double));
}


// data_seqv_writer_big_endian_tpl
template<class T, class writer> void data_seqv_writer_big_endian_tpl<T, writer>::write_bytes(const std::byte* i_seqv, size_t i_elem_count, size_t i_offset)
{
   writer_v.write_bytes(i_seqv, i_elem_count, i_offset);
}


// data_seqv_writer_base_tpl
template<class T, class writer> data_seqv_writer_base_tpl<T, writer>& data_seqv_writer_base_tpl<T, writer>::operator=(data_seqv_writer_base_tpl<T, writer>&& i_seqv) noexcept
{
   if (this != &i_seqv)
   {
      seqv_v = std::move(i_seqv.seqv_v);
   }

   return *this;
}

template<class T, class writer> T& data_seqv_writer_base_tpl<T, writer>::dsv() { return seqv_v; }
template<class T, class writer> const T& data_seqv_writer_base_tpl<T, writer>::dsv() const { return seqv_v; }
template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_byte(std::byte i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_char(char i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_i8(int8_t i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_u8(uint8_t i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_i16(int16_t i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_u16(uint16_t i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_i32(int32_t i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_u32(uint32_t i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_i64(int64_t i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_u64(uint64_t i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_f32(float i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_f64(double i_seqv) { write(i_seqv); }
template<class T, class writer> template<class T0> void data_seqv_writer_base_tpl<T, writer>::write(const T0& i_data) { write_bytes(byte_const_cast(&i_data), sizeof(T0)); }

template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_line(const std::string& i_text, bool i_new_line)
{
   write_bytes(byte_const_cast(i_text.data()), i_text.length(), 0);
   if (i_new_line) { write<char>('\n'); }
}

template<class T, class writer> template<class T0> void data_seqv_writer_base_tpl<T, writer>::write_pointer(T0* const i_seqv)
{
   write_bytes(byte_const_cast(&i_seqv), sizeof(uintptr_t), 0);
}

template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_bytes(const std::byte* i_seqv, size_t i_elem_count, size_t i_offset)
{
   writer()(seqv_v)->write_bytes(i_seqv, i_elem_count, i_offset);
}

template<class T, class writer> template<class T0> void data_seqv_writer_base_tpl<T, writer>::
write_sized_byte_vect(const std::byte* i_seqv, T0 i_elem_count, T0 i_offset)
{
   assert(i_elem_count <= std::numeric_limits<T0>::max());
   write<T0>(i_elem_count);
   write_bytes(i_seqv, i_elem_count, i_offset);
}

template<class T, class writer> template<class T0> void data_seqv_writer_base_tpl<T, writer>::write_sized_text(const std::string& i_text)
{
   assert(i_text.length() <= std::numeric_limits<T0>::max());
   write<T0>(static_cast<T0>(i_text.length()));
   write_bytes(byte_const_cast(i_text.data()), i_text.length(), 0);
}

template<class T, class writer> template<class T0> void data_seqv_writer_base_tpl<T, writer>::write(const T0* i_seqv, size_t i_elem_count, size_t i_offset)
{
   write_bytes(byte_const_cast(i_seqv), i_elem_count * sizeof(T0), i_offset * sizeof(T0));
}

template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_chars(const char* i_seqv, size_t i_elem_count, size_t i_offset)
{
   if (i_elem_count == 0) { i_elem_count = strlen(i_seqv); }
   write_bytes(byte_const_cast(i_seqv), i_elem_count, i_offset);
}

template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_string(const std::string& i_seqv)
{
   write_bytes(byte_const_cast(i_seqv.c_str()), i_seqv.length());
}

template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_i8(const int8_t* i_seqv, size_t i_elem_count, size_t i_offset)
{
   write_bytes(byte_const_cast(i_seqv), i_elem_count, i_offset);
}

template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_u8(const uint8_t* i_seqv, size_t i_elem_count, size_t i_offset)
{
   write_bytes(byte_const_cast(i_seqv), i_elem_count, i_offset);
}

template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_i16(const int16_t* i_seqv, size_t i_elem_count, size_t i_offset)
{
   write_bytes(byte_const_cast(i_seqv), i_elem_count * sizeof(int16_t), i_offset * sizeof(int16_t));
}

template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_u16(const uint16_t* i_seqv, size_t i_elem_count, size_t i_offset)
{
   write_bytes(byte_const_cast(i_seqv), i_elem_count * sizeof(uint16_t), i_offset * sizeof(uint16_t));
}

template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_i32(const int32_t* i_seqv, size_t i_elem_count, size_t i_offset)
{
   write_bytes(byte_const_cast(i_seqv), i_elem_count * sizeof(int32_t), i_offset * sizeof(int32_t));
}

template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_u32(const uint32_t* i_seqv, size_t i_elem_count, size_t i_offset)
{
   write_bytes(byte_const_cast(i_seqv), i_elem_count * sizeof(uint32_t), i_offset * sizeof(uint32_t));
}

template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_i64(const int64_t* i_seqv, size_t i_elem_count, size_t i_offset)
{
   write_bytes(byte_const_cast(i_seqv), i_elem_count * sizeof(int64_t), i_offset * sizeof(int64_t));
}

template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_u64(const uint64_t* i_seqv, size_t i_elem_count, size_t i_offset)
{
   write_bytes(byte_const_cast(i_seqv), i_elem_count * sizeof(uint64_t), i_offset * sizeof(uint64_t));
}

template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_f32(const float* i_seqv, size_t i_elem_count, size_t i_offset)
{
   write_bytes(byte_const_cast(i_seqv), i_elem_count * sizeof(float), i_offset * sizeof(float));
}

template<class T, class writer> void data_seqv_writer_base_tpl<T, writer>::write_f64(const double* i_seqv, size_t i_elem_count, size_t i_offset)
{
   write_bytes(byte_const_cast(i_seqv), i_elem_count * sizeof(double), i_offset * sizeof(double));
}


// data_seqv_mem_writer_tpl
template<class byte_seqv> data_seqv_mem_writer_tpl<byte_seqv>& data_seqv_mem_writer_tpl<byte_seqv>::operator=(const data_seqv_mem_writer_tpl& i_seqv)
{
   if (this != &i_seqv)
   {
      this->seqv_v = i_seqv.seqv_v;
   }

   return *this;
}

template<class byte_seqv> data_seqv_mem_writer_tpl<byte_seqv>& data_seqv_mem_writer_tpl<byte_seqv>::operator=(const data_seqv_rw_mem_tpl<byte_seqv>& i_seqv)
{
   if (&(this->seqv_v) != &i_seqv)
   {
      this->seqv_v = i_seqv;
   }

   return *this;
}

template<class byte_seqv> data_seqv_mem_writer_tpl<byte_seqv>& data_seqv_mem_writer_tpl<byte_seqv>::operator=(const byte_seqv& i_seqv)
{
   this->seqv_v = i_seqv;
   return *this;
}

template<class byte_seqv> data_seqv_mem_writer_tpl<byte_seqv>& data_seqv_mem_writer_tpl<byte_seqv>::operator=(data_seqv_mem_writer_tpl&& i_seqv) noexcept
{
   if (this != &i_seqv)
   {
      data_seqv_writer_base_tpl<data_seqv_rw_mem_tpl<byte_seqv>, ref_adapter<data_seqv_tpl<byte_seqv>>>::operator=(std::move(i_seqv));
   }

   return *this;
}

template<class byte_seqv> data_seqv_mem_writer_tpl<byte_seqv>& data_seqv_mem_writer_tpl<byte_seqv>::operator=(data_seqv_rw_mem_tpl<byte_seqv>&& i_seqv) noexcept
{
   if (&(this->seqv_v) != &i_seqv)
   {
      this->seqv_v = std::move(i_seqv);
   }

   return *this;
}

template<class byte_seqv> data_seqv_mem_writer_tpl<byte_seqv>& data_seqv_mem_writer_tpl<byte_seqv>::operator=(byte_seqv&& i_seqv) noexcept
{
   this->seqv_v = std::move(i_seqv);
   return *this;
}


// data_seqv_exception
inline data_seqv_exception::data_seqv_exception() { set_msg(""); }
inline data_seqv_exception::data_seqv_exception(const std::string& i_msg) { set_msg(i_msg.c_str()); }
inline data_seqv_exception::data_seqv_exception(const char* i_msg) { set_msg(i_msg); }
inline data_seqv_exception::~data_seqv_exception() {}
inline const char* data_seqv_exception::what() const noexcept { return msg.c_str(); }
inline void data_seqv_exception::throw_ex(const char* i_msg) { mws_throw data_seqv_exception(i_msg); }

inline void data_seqv_exception::set_msg(const char* i_msg)
{
   msg = i_msg;

#ifndef __cpp_exceptions
   assert(false);
#endif
}
