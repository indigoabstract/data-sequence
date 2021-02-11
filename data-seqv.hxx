#pragma once

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <memory>
#include <vector>
#include <string>


// type conversion utils: type aliasing/punning and reference/pointer converters
template<class T> std::byte* byte_cast(T* i_addr) { return reinterpret_cast<std::byte*>(i_addr); }
template<class T> const std::byte* byte_const_cast(const T* i_addr) { return reinterpret_cast<const std::byte*>(i_addr); }
template<class T> struct ptr_adapter { T operator() (T& i_obj) { return i_obj; } const T operator() (const T& i_obj) { return i_obj; } };
template<class T> struct ref_adapter { T* operator() (T& i_obj) { return &i_obj; } const T* operator() (const T& i_obj) { return &i_obj; } };


/** interface/base class for all data sequences. note: writing to the sequence does not change the read position and viceversa */
class data_seqv
{
public:
   data_seqv();
   data_seqv(const data_seqv& i_seqv) { operator=(i_seqv); }
   data_seqv(data_seqv&& i_seqv) noexcept { operator=(std::move(i_seqv)); }
   virtual ~data_seqv() {}
   data_seqv& operator=(const data_seqv& i_seqv);
   data_seqv& operator=(data_seqv&& i_seqv) noexcept;
   /** returns true when there are no more bytes to read */
   virtual bool is_end_of_seqv();
   virtual void close();
   /** returns the sequence data as a byte array */
   virtual const std::byte* seqv_as_array() const = 0;
   /** returns the sequence data as a byte vector */
   virtual std::vector<std::byte> seqv_as_vector() const = 0;
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
   int read_bytes(std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   /** returns number of bytes written */
   int write_bytes(const std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);

protected:
   /** returns number of bytes read */
   virtual int read_bytes_impl(std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset) = 0;
   /** returns number of bytes written */
   virtual int write_bytes_impl(const std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset) = 0;

   uint64_t read_position_v;
   uint64_t write_position_v;
   uint64_t total_bytes_read_v;
   uint64_t total_bytes_written_v;
};


/** read only memory data sequence. doesn't copy the input data, but only stores a reference. be very careful with this.
note: writing to the sequence does not change the read position and viceversa */
class data_seqv_ro_mem : public data_seqv
{
public:
   /** constructs an empty ro memory data sequence */
   data_seqv_ro_mem() {}
   /** constructs a ro memory data sequence with a pointer to data copied from i_seqv. i_seqv's size must NOT be modified while referenced by data_seqv_ro_mem */
   data_seqv_ro_mem(const data_seqv& i_seqv);
   /** constructs a ro memory data sequence with a pointer to data copied from i_seqv. i_seqv's size must NOT be modified while referenced by data_seqv_ro_mem */
   data_seqv_ro_mem(const std::vector<std::byte>& i_seqv);
   /** constructs a ro memory data sequence with a pointer to data copied from i_seqv. i_seqv's size must NOT be modified while referenced by data_seqv_ro_mem */
   data_seqv_ro_mem(const std::byte* i_seqv, uint64_t i_elem_count);
   virtual ~data_seqv_ro_mem() {}
   virtual const std::byte* seqv_as_array() const override;
   virtual std::vector<std::byte> seqv_as_vector() const override;
   virtual uint64_t size() const override;
   virtual bool is_writable() const { return false; }
   virtual void rewind() override;
   virtual void reset() override;
   virtual void set_io_position(uint64_t i_position) override;
   virtual void set_read_position(uint64_t i_position) override;
   virtual void set_write_position(uint64_t i_position) override;

protected:
   virtual int read_bytes_impl(std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset) override;
   virtual int write_bytes_impl(const std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset) override;

   /** size of this sequence */
   uint64_t size_v = 0;
   /** note: reference only, does not own the memory that it points to */
   const std::byte* seqv = nullptr;
};


/** read/write memory data sequence. note: writing to the sequence does not change the read position and viceversa */
class data_seqv_rw_mem : public data_seqv
{
public:
   /** constructs an empty memory data sequence */
   data_seqv_rw_mem() {}
   /** constructs a memory data sequence with its size the specified number of bytes */
   data_seqv_rw_mem(uint32_t i_elem_count) : seqv(i_elem_count) {}
   /** constructs a memory data sequence with data copied from i_seqv */
   data_seqv_rw_mem(const std::vector<std::byte>& i_seqv) : seqv(i_seqv) {}
   /** constructs a memory data sequence with data moved from i_seqv */
   data_seqv_rw_mem(std::vector<std::byte>&& i_seqv) : seqv(std::move(i_seqv)) {}
   /** constructs a memory data sequence with data copied from i_seqv */
   data_seqv_rw_mem(const std::byte* i_seqv, uint32_t i_elem_count);
   /** constructs a memory data sequence with data copied from i_seqv */
   data_seqv_rw_mem(const data_seqv_rw_mem& i_seqv) : data_seqv(i_seqv), seqv(i_seqv.seqv) {}
   /** constructs a memory data sequence with data moved from i_seqv */
   data_seqv_rw_mem(data_seqv_rw_mem&& i_seqv) noexcept : data_seqv(std::move(i_seqv)), seqv(std::move(i_seqv.seqv)) {}
   virtual ~data_seqv_rw_mem() {}
   data_seqv_rw_mem& operator=(const data_seqv_rw_mem& i_seqv);
   data_seqv_rw_mem& operator=(data_seqv_rw_mem&& i_seqv) noexcept;
   virtual uint64_t size() const override;
   virtual void rewind() override;
   virtual void reset() override;
   virtual const std::byte* seqv_as_array() const override;
   virtual std::vector<std::byte> seqv_as_vector() const override;
   virtual void set_io_position(uint64_t i_position) override;
   virtual void set_read_position(uint64_t i_position) override;
   virtual void set_write_position(uint64_t i_position) override;
   /** resizes this sequence to the specified number of bytes */
   virtual void resize(uint32_t i_elem_count);
   /** moves the data in this sequence into the byte vector i_seqv. this object is left empty */
   virtual void move_into(std::vector<std::byte>& i_seqv);

protected:
   virtual int read_bytes_impl(std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset) override;
   virtual int write_bytes_impl(const std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset) override;

   std::vector<std::byte> seqv;
};


// data_seqv_file_base
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
   virtual int read_bytes(std::byte* i_seqv, uint32_t i_size, uint32_t i_offset = 0) = 0;
   virtual int write_bytes(const std::byte* i_seqv, uint32_t i_size, uint32_t i_offset = 0) = 0;
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
   virtual int read_bytes(std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0) override;
   virtual int write_bytes(const std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0) override;
   virtual std::FILE* file_ptr() const;

protected:
   std::shared_ptr<std::FILE> file_v;
   bool is_writable_v;
};


/** base class for a file data sequence. note: writing to the sequence does not change the read position and viceversa */
template<class T, class io> class data_seqv_file_base : public data_seqv
{
public:
   data_seqv_file_base(T i_file) : file_v(i_file) {}
   virtual ~data_seqv_file_base() {}
   virtual bool is_end_of_seqv() override;
   virtual void close() override;
   virtual const std::byte* seqv_as_array() const override { return nullptr; }
   virtual std::vector<std::byte> seqv_as_vector() const override { return std::vector<std::byte>(); }
   virtual uint64_t size() const override;
   virtual bool is_writable() const override { return io()(file_v)->is_writable(); }
   virtual void rewind() override;
   virtual void reset() override;
   virtual void set_io_position(uint64_t i_position) override;
   virtual void set_read_position(uint64_t i_position) override;
   virtual void set_write_position(uint64_t i_position) override;
   virtual const T& file() const;

protected:
   virtual int read_bytes_impl(std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset) override;
   virtual int write_bytes_impl(const std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset) override;

   uint64_t last_file_pos = 0;
   T file_v;
};


/** file sequence reference version */
class data_seqv_file : public data_seqv_file_base<data_seqv_std_file_wrapper, ref_adapter<data_seqv_std_file_wrapper>>
{
public:
   data_seqv_file(const data_seqv_std_file_wrapper& i_file) : data_seqv_file_base(i_file)
   {
      assert(i_file.is_open());
   }
   virtual void set_file_wrapper(const data_seqv_std_file_wrapper& i_file) { file_v = i_file; }
};


/** file sequence pointer version */
class data_seqv_file_ptr : public data_seqv_file_base<data_seqv_file_wrapper*, ptr_adapter<data_seqv_file_wrapper*>>
{
public:
   data_seqv_file_ptr() : data_seqv_file_base(nullptr) {}
   data_seqv_file_ptr(data_seqv_file_wrapper* i_file) : data_seqv_file_base(i_file) { assert(!i_file || i_file->is_open()); }
   virtual void set_file_wrapper(data_seqv_file_wrapper* i_file) { file_v = i_file; }
};


/** file sequence shared pointer version */
class data_seqv_file_shr : public data_seqv_file_base<std::shared_ptr<data_seqv_file_wrapper>, ptr_adapter<std::shared_ptr<data_seqv_file_wrapper>>>
{
public:
   data_seqv_file_shr() : data_seqv_file_base(nullptr) {}
   data_seqv_file_shr(std::shared_ptr<data_seqv_file_wrapper> i_file) : data_seqv_file_base(i_file) { assert(!i_file || i_file->is_open()); }
   virtual void set_file_wrapper(std::shared_ptr<data_seqv_file_wrapper> i_file) { file_v = i_file; }
};


/** base class for data sequence readers */
template<class T, class reader> class data_seqv_reader_base
{
public:
   data_seqv_reader_base(T i_seqv) : seqv(i_seqv) {}
   data_seqv_reader_base(const data_seqv_reader_base& i_seqv) : seqv(i_seqv.seqv) {}
   data_seqv_reader_base(data_seqv_reader_base&& i_seqv) noexcept : seqv(std::move(i_seqv.seqv)) {}
   virtual ~data_seqv_reader_base() {}
   data_seqv_reader_base& operator=(const data_seqv_reader_base& i_seqv) = delete;
   data_seqv_reader_base& operator=(data_seqv_reader_base&& i_seqv) noexcept;
   /** returns the underlying data sequence */
   T& dsv();
   /** returns the underlying data sequence */
   const T& dsv() const;
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
   template<class T0> T0 read();
   /** avoid using read_line(), as it's quite slow */
   std::string read_line();
   template<class T0> void read_pointer(T0*& i_seqv);
   // seqv data versions. each returns the number of bytes read
   int read_bytes(std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   /** reads a vector of bytes of length 'i_elem_count' */
   std::vector<std::byte> read_byte_vect(uint32_t i_elem_count);
   /** reads a sequence that was written with the write_sized_byte_vect() function.
   it's the same as read_byte_vect(), but the size is read from the sequence */
   std::vector<std::byte> read_sized_byte_vect();
   /** reads a text(string) that was written with the write_sized_text() function */
   std::string read_sized_text();
   template<class T0> int read(T0* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   int read_chars(char* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   std::string read_string(uint32_t i_elem_count);
   int read_i8(int8_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   int read_u8(uint8_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   int read_i16(int16_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   int read_u16(uint16_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   int read_i32(int32_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   int read_u32(uint32_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   int read_i64(int64_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   int read_u64(uint64_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   int read_f32(float* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   int read_f64(double* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);

protected:
   T seqv;
};


/** ro memory sequence reader embedded instance version */
class data_seqv_ro_mem_reader : public data_seqv_reader_base<data_seqv_ro_mem, ref_adapter<data_seqv>>
{
public:
   /** constructs a ro memory data sequence with a pointer to data copied from i_seqv. i_seqv's size must NOT be modified while referenced by data_seqv_ro_mem */
   data_seqv_ro_mem_reader(data_seqv_ro_mem i_seqv) : data_seqv_reader_base(i_seqv) {}
   /** constructs a ro memory data sequence with a pointer to data copied from i_seqv. i_seqv's size must NOT be modified while referenced by data_seqv_ro_mem */
   data_seqv_ro_mem_reader(const std::byte* i_data, uint32_t i_size) : data_seqv_reader_base(data_seqv_ro_mem(i_data, i_size)) {}
};

/** ro memory sequence reader reference version */
class data_seqv_ro_mem_reader_ref : public data_seqv_reader_base<data_seqv_ro_mem&, ref_adapter<data_seqv>>
{
public:
   /** constructs a ro memory data sequence with a pointer to data copied from i_seqv. i_seqv's size must NOT be modified while referenced by data_seqv_ro_mem */
   data_seqv_ro_mem_reader_ref(data_seqv_ro_mem& i_seqv) : data_seqv_reader_base(i_seqv) {}
};


/** memory sequence reader reference version */
class data_seqv_mem_reader_ref : public data_seqv_reader_base<data_seqv_rw_mem&, ref_adapter<data_seqv>>
{
public:
   /** constructs a rw memory data sequence with a reference to i_seqv */
   data_seqv_mem_reader_ref(data_seqv_rw_mem& i_seqv) : data_seqv_reader_base(i_seqv) {}
};


/** file sequence reader reference version */
class data_seqv_file_reader_ref : public data_seqv_reader_base<data_seqv_file&, ref_adapter<data_seqv>>
{
public:
   /** constructs a file data sequence with data copied from i_seqv */
   data_seqv_file_reader_ref(data_seqv_file& i_seqv) : data_seqv_reader_base(i_seqv) {}
};


/** sequence reader pointer version */
class data_seqv_reader_ptr : public data_seqv_reader_base<data_seqv*, ptr_adapter<data_seqv*>>
{
public:
   data_seqv_reader_ptr() : data_seqv_reader_base(nullptr) {}
   data_seqv_reader_ptr(data_seqv* i_seqv) : data_seqv_reader_base(i_seqv) {}
   void set_data_sequence(data_seqv* i_seqv) { seqv = i_seqv; }
};


/** sequence reader shared pointer version */
class data_seqv_reader_shr : public data_seqv_reader_base<std::shared_ptr<data_seqv>, ptr_adapter<std::shared_ptr<data_seqv>>>
{
public:
   data_seqv_reader_shr() : data_seqv_reader_base(nullptr) {}
   data_seqv_reader_shr(std::shared_ptr<data_seqv> i_seqv) : data_seqv_reader_base(i_seqv) {}
   void set_data_sequence(std::shared_ptr<data_seqv> i_seqv) { seqv = i_seqv; }
};


/** base class for data sequence writers */
template<class T, class writer> class data_seqv_writer_base
{
public:
   data_seqv_writer_base(T i_seqv) : seqv(i_seqv) {}
   data_seqv_writer_base(const data_seqv_writer_base& i_seqv) : seqv(i_seqv.seqv) {}
   data_seqv_writer_base(data_seqv_writer_base&& i_seqv) noexcept : seqv(std::move(i_seqv.seqv)) {}
   virtual ~data_seqv_writer_base() {}
   data_seqv_writer_base& operator=(const data_seqv_writer_base& i_seqv) = delete;
   data_seqv_writer_base& operator=(data_seqv_writer_base&& i_seqv) noexcept;
   /** returns the underlying data sequence */
   T& dsv();
   /** returns the underlying data sequence */
   const T& dsv() const;
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
   template<class T0> void write(const T0& i_data);
   void write_line(const std::string& i_text, bool i_new_line = true);
   template<class T0> void write_pointer(T0* const i_seqv);
   // seqv data versions
   void write_bytes(const std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   /** writes a vector of bytes */
   void write_byte_vect(const std::vector<std::byte>& i_seqv) { write_bytes(i_seqv.data(), i_seqv.size()); }
   /** same as write_byte_vect, but also writes in front the number of bytes in the sequence(a 32 bit number) */
   void write_sized_byte_vect(const std::vector<std::byte>& i_seqv) { write_sized_byte_vect(i_seqv.data(), i_seqv.size()); }
   void write_sized_byte_vect(const std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   /** writes a text(string) and also writes in front the number of bytes in the sequence(a 32 bit number) */
   void write_sized_text(const std::string& i_text);
   template<class T0> void write(const T0* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   void write_chars(const char* i_seqv, uint32_t i_elem_count = 0, uint32_t i_offset = 0);
   void write_string(const std::string& i_seqv);
   void write_i8(const int8_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   void write_u8(const uint8_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   void write_i16(const int16_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   void write_u16(const uint16_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   void write_i32(const int32_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   void write_u32(const uint32_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   void write_i64(const int64_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   void write_u64(const uint64_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   void write_f32(const float* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   void write_f64(const double* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);

protected:
   T seqv;
};


/** memory sequence writer embedded instance version */
class data_seqv_mem_writer : public data_seqv_writer_base<data_seqv_rw_mem, ref_adapter<data_seqv>>
{
public:
   /** constructs an empty rw memory data sequence */
   data_seqv_mem_writer() : data_seqv_writer_base(data_seqv_rw_mem()) {}
   /** constructs a rw memory data sequence with its size the specified number of bytes */
   data_seqv_mem_writer(uint32_t i_elem_count) : data_seqv_writer_base(data_seqv_rw_mem()) { dsv().resize(i_elem_count); }
   /** constructs a rw memory data sequence with data copied from i_seqv */
   data_seqv_mem_writer(const data_seqv_mem_writer& i_seqv) : data_seqv_writer_base(i_seqv) {}
   /** constructs a rw memory data sequence with data moved from i_seqv */
   data_seqv_mem_writer(data_seqv_mem_writer&& i_seqv) noexcept : data_seqv_writer_base(std::move(i_seqv)) {}
   data_seqv_mem_writer& operator=(const data_seqv_mem_writer& i_seqv);
   data_seqv_mem_writer& operator=(data_seqv_mem_writer&& i_seqv) noexcept;
};


/** memory sequence writer reference version */
class data_seqv_mem_writer_ref : public data_seqv_writer_base<data_seqv_rw_mem&, ref_adapter<data_seqv>>
{
public:
   /** constructs a rw memory data sequence with a reference to i_seqv */
   data_seqv_mem_writer_ref(data_seqv_rw_mem& i_seqv) : data_seqv_writer_base(i_seqv) {}
};


/** file sequence writer reference version */
class data_seqv_file_writer_ref : public data_seqv_writer_base<data_seqv_file&, ref_adapter<data_seqv>>
{
public:
   /** constructs a file data sequence with data copied from i_seqv */
   data_seqv_file_writer_ref(data_seqv_file& i_seqv) : data_seqv_writer_base(i_seqv) { assert(i_seqv.is_writable()); }
};


/** sequence writer pointer version */
class data_seqv_writer_ptr : public data_seqv_writer_base<data_seqv*, ptr_adapter<data_seqv*>>
{
public:
   data_seqv_writer_ptr() : data_seqv_writer_base(nullptr) {}
   data_seqv_writer_ptr(data_seqv* i_seqv) : data_seqv_writer_base(i_seqv) { assert(i_seqv->is_writable()); }
   void set_data_sequence(data_seqv* i_seqv) { assert(i_seqv->is_writable()); seqv = i_seqv; }
};


/** sequence writer shared pointer version */
class data_seqv_writer_shr : public data_seqv_writer_base<std::shared_ptr<data_seqv>, ptr_adapter<std::shared_ptr<data_seqv>>>
{
public:
   data_seqv_writer_shr() : data_seqv_writer_base(nullptr) {}
   data_seqv_writer_shr(std::shared_ptr<data_seqv> i_seqv) : data_seqv_writer_base(i_seqv) { assert(i_seqv->is_writable()); }
   void set_data_sequence(std::shared_ptr<data_seqv> i_seqv) { assert(i_seqv->is_writable()); assert(i_seqv->is_writable()); seqv = i_seqv; }
};


/** implements read/write operations on a memory data sequence */
class data_seqv_rw_mem_ops : public data_seqv_rw_mem
{
public:
   /** constructs an empty rw memory data sequence */
   data_seqv_rw_mem_ops() : r(*this), w(*this) {}
   /** constructs a rw memory data sequence with its size the specified number of bytes */
   data_seqv_rw_mem_ops(uint32_t i_elem_count) : data_seqv_rw_mem(i_elem_count), r(*this), w(*this) {}
   /** constructs a rw memory data sequence with data copied from i_seqv */
   data_seqv_rw_mem_ops(const data_seqv_rw_mem& i_seqv) : data_seqv_rw_mem(i_seqv), r(*this), w(*this) {}
   /** constructs a rw memory data sequence with data moved from i_seqv */
   data_seqv_rw_mem_ops(data_seqv_rw_mem&& i_seqv) noexcept : data_seqv_rw_mem(std::move(i_seqv)), r(*this), w(*this) {}

   data_seqv_mem_reader_ref r;
   data_seqv_mem_writer_ref w;
};


/** implements read/write operations on a file data sequence */
class data_seqv_rw_file_ops : public data_seqv_file
{
public:
   data_seqv_rw_file_ops(const data_seqv_std_file_wrapper& i_file) : data_seqv_file(i_file), r(*this), w(*this) { assert(i_file.is_writable()); }

   data_seqv_file_reader_ref r;
   data_seqv_file_writer_ref w;
};


/** data_seqv_reader_big_endian. work in progress */
class data_seqv_reader_big_endian
{
public:
   data_seqv_reader_big_endian(data_seqv* i_seqv) : seqv(i_seqv) {}
   ~data_seqv_reader_big_endian() {}
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
   // seqv data versions
   void read_i8(int8_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   void read_u8(uint8_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   //void read_i16(int16_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   //void read_u16(uint16_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   //void read_i32(int32_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   //void read_u32(uint32_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   //void read_i64(int64_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   //void read_u64(uint64_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   //void read_f32(float* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);
   //void read_f64(double* i_seqv, uint32_t i_elem_count, uint32_t i_offset = 0);

private:
   data_seqv* seqv;
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










// data_seqv
inline data_seqv::data_seqv() { data_seqv::reset(); }

inline data_seqv& data_seqv::operator=(const data_seqv& i_seqv)
{
   if (this != &i_seqv)
   {
      read_position_v = i_seqv.read_position_v;
      write_position_v = i_seqv.write_position_v;
      total_bytes_read_v = i_seqv.total_bytes_read_v;
      total_bytes_written_v = i_seqv.total_bytes_written_v;
   }
   return *this;
}

inline data_seqv& data_seqv::operator=(data_seqv&& i_seqv) noexcept
{
   if (this != &i_seqv)
   {
      read_position_v = i_seqv.read_position_v;
      write_position_v = i_seqv.write_position_v;
      total_bytes_read_v = i_seqv.total_bytes_read_v;
      total_bytes_written_v = i_seqv.total_bytes_written_v;
      i_seqv.data_seqv::reset();
   }
   return *this;
}

inline bool data_seqv::is_end_of_seqv() { return read_position() >= size(); }
inline void data_seqv::close() {}
inline uint64_t data_seqv::read_position() const { return read_position_v; }
inline uint64_t data_seqv::write_position() const { return write_position_v; }
inline uint64_t data_seqv::total_bytes_read() const { return total_bytes_read_v; }
inline uint64_t data_seqv::total_bytes_written() const { return total_bytes_written_v; }
inline void data_seqv::rewind() { read_position_v = write_position_v = total_bytes_read_v = total_bytes_written_v = 0; }
inline void data_seqv::reset() { rewind(); }
inline void data_seqv::set_io_position(uint64_t i_position) { read_position_v = write_position_v = i_position; }
inline void data_seqv::set_read_position(uint64_t i_position) { read_position_v = i_position; }
inline void data_seqv::set_write_position(uint64_t i_position) { write_position_v = i_position; }

inline int data_seqv::read_bytes(std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   int bytes_read = read_bytes_impl(i_seqv, i_elem_count, i_offset);

   read_position_v += bytes_read;
   total_bytes_read_v += bytes_read;

   return bytes_read;
}

inline int data_seqv::write_bytes(const std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   int bytes_written = write_bytes_impl(i_seqv, i_elem_count, i_offset);

   write_position_v += bytes_written;
   total_bytes_written_v += bytes_written;

   return bytes_written;
}


// data_seqv_ro_mem
inline data_seqv_ro_mem::data_seqv_ro_mem(const data_seqv& i_seqv) : seqv(i_seqv.seqv_as_array()), size_v(i_seqv.size()) {}
inline data_seqv_ro_mem::data_seqv_ro_mem(const std::vector<std::byte>& i_seqv) : seqv(i_seqv.data()), size_v(i_seqv.size()) {}
inline data_seqv_ro_mem::data_seqv_ro_mem(const std::byte* i_seqv, uint64_t i_elem_count) : seqv(i_seqv), size_v(i_elem_count) {}
inline const std::byte* data_seqv_ro_mem::seqv_as_array() const { return seqv; }
inline uint64_t data_seqv_ro_mem::size() const { return size_v; }
inline void data_seqv_ro_mem::rewind() { data_seqv::rewind(); }
inline void data_seqv_ro_mem::reset() { data_seqv::reset(); }
inline void data_seqv_ro_mem::set_io_position(uint64_t i_position) { set_read_position(i_position); }

inline std::vector<std::byte> data_seqv_ro_mem::seqv_as_vector() const
{
   std::vector<std::byte> sq;
   size_t sz = static_cast<size_t>(size());

   if (sz > 0)
   {
      sq.assign(seqv, seqv + sz);
   }

   return sq;
}

inline void data_seqv_ro_mem::set_read_position(uint64_t i_pos) { if (i_pos > size()) { data_seqv_exception::throw_ex(); } else { read_position_v = i_pos; } }
inline void data_seqv_ro_mem::set_write_position(uint64_t) { data_seqv_exception::throw_ex(); }

inline int data_seqv_ro_mem::read_bytes_impl(std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   int bytes_to_read = 0;

   if (read_position() < size())
   {
      bytes_to_read = (int)std::min((uint64_t)i_elem_count, size() - read_position());
      std::memcpy(&i_seqv[i_offset], &seqv[(size_t)read_position()], bytes_to_read);
   }

   return bytes_to_read;
}

inline int data_seqv_ro_mem::write_bytes_impl(const std::byte*, uint32_t, uint32_t) { data_seqv_exception::throw_ex(); return -1; }


// data_seqv_rw_mem
inline data_seqv_rw_mem::data_seqv_rw_mem(const std::byte* i_seqv, uint32_t i_elem_count) { seqv.assign(i_seqv, i_seqv + i_elem_count); }

inline data_seqv_rw_mem& data_seqv_rw_mem::operator=(const data_seqv_rw_mem& i_seqv)
{
   if (this != &i_seqv)
   {
      seqv = i_seqv.seqv;
   }
   return *this;
}

inline data_seqv_rw_mem& data_seqv_rw_mem::operator=(data_seqv_rw_mem&& i_seqv) noexcept
{
   if (this != &i_seqv)
   {
      data_seqv::operator=(std::move(i_seqv));
      seqv = std::move(i_seqv.seqv);
   }
   return *this;
}

inline const std::byte* data_seqv_rw_mem::seqv_as_array() const { return seqv.data(); }
inline std::vector<std::byte> data_seqv_rw_mem::seqv_as_vector() const { return seqv; }
inline uint64_t data_seqv_rw_mem::size() const { return seqv.size(); }
inline void data_seqv_rw_mem::rewind() { data_seqv::rewind(); }
inline void data_seqv_rw_mem::reset() { data_seqv::reset(); seqv.clear(); }
inline void data_seqv_rw_mem::set_io_position(uint64_t i_position) { set_read_position(i_position); set_write_position(i_position); }
inline void data_seqv_rw_mem::set_read_position(uint64_t i_pos) { if (i_pos > size()) { data_seqv_exception::throw_ex(); } else { read_position_v = i_pos; } }
inline void data_seqv_rw_mem::set_write_position(uint64_t i_pos) { if (i_pos > size()) { data_seqv_exception::throw_ex(); } else { write_position_v = i_pos; } }

inline void data_seqv_rw_mem::resize(uint32_t i_elem_count)
{
   seqv.resize(i_elem_count);
   if (read_position_v >= i_elem_count) { set_read_position(i_elem_count); }
   if (write_position_v >= i_elem_count) { set_write_position(i_elem_count); }
}

inline void data_seqv_rw_mem::move_into(std::vector<std::byte>& i_seqv)
{
   i_seqv.swap(seqv);
   reset();
}

inline int data_seqv_rw_mem::read_bytes_impl(std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   int bytes_to_read = 0;

   if (read_position() < size())
   {
      bytes_to_read = (int)std::min((uint64_t)i_elem_count, size() - read_position());
      std::memcpy(&i_seqv[i_offset], &seqv[(size_t)read_position()], bytes_to_read);
   }

   return bytes_to_read;
}

inline int data_seqv_rw_mem::write_bytes_impl(const std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   if (size() - write_position() < i_elem_count)
   {
      seqv.resize((size_t)size() + i_elem_count);
   }

   std::memcpy(&seqv[(size_t)write_position()], &i_seqv[i_offset], i_elem_count);

   return i_elem_count;
}


// data_seqv_file_base
// data_seqv_std_file_wrapper
inline data_seqv_std_file_wrapper::data_seqv_std_file_wrapper(const std::string& i_file_path, const std::string& i_open_mode)
{
   const size_t np = std::string::npos;
   is_writable_v = (i_open_mode.find('a') != np) || (i_open_mode.find('w') != np) || (i_open_mode.find('+') != np);
#pragma warning(suppress : 4996)
   std::FILE* f = fopen(i_file_path.c_str(), i_open_mode.c_str());
   if (f) { file_v = std::shared_ptr<std::FILE>(f, std::fclose); }
}

inline data_seqv_std_file_wrapper::data_seqv_std_file_wrapper() : is_writable_v(false) {}
inline data_seqv_std_file_wrapper::data_seqv_std_file_wrapper(std::shared_ptr<std::FILE> i_file, bool i_is_writable) : file_v(i_file), is_writable_v(i_is_writable) {}
inline bool data_seqv_std_file_wrapper::is_open() const { return file_v != nullptr; }
inline bool data_seqv_std_file_wrapper::is_writable() const { return is_writable_v; }
inline uint64_t data_seqv_std_file_wrapper::length() const { std::FILE* f = file_ptr(); fseek(f, 0L, SEEK_END); long size = ftell(f); rewind(f); return size; }
inline void data_seqv_std_file_wrapper::close() { fclose(file_ptr()); }
inline void data_seqv_std_file_wrapper::set_io_position(uint64_t i_position) { fseek(file_ptr(), static_cast<long>(i_position), 0); }
inline std::FILE* data_seqv_std_file_wrapper::file_ptr() const { return file_v.get(); }

inline int data_seqv_std_file_wrapper::read_bytes(std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   return fread(i_seqv + i_offset, 1, i_elem_count, file_ptr());
}

inline int data_seqv_std_file_wrapper::write_bytes(const std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   return fwrite(i_seqv + i_offset, 1, i_elem_count, file_ptr());
}


// data_seqv_file_base
template<class T, class io> bool data_seqv_file_base<T, io>::is_end_of_seqv()
{
   uint64_t file_size = size();
   if (write_position() > file_size) { return read_position() >= write_position(); }
   return read_position() >= file_size;
}

template<class T, class io> void data_seqv_file_base<T, io>::close() { io()(file_v)->close(); }
template<class T, class io> uint64_t data_seqv_file_base<T, io>::size() const { return io()(file_v)->length(); }
template<class T, class io> void data_seqv_file_base<T, io>::rewind() { data_seqv::rewind(); set_io_position(0); }
template<class T, class io> void data_seqv_file_base<T, io>::reset() { data_seqv::reset(); set_io_position(0); }
template<class T, class io> const T& data_seqv_file_base<T, io>::file() const { return file_v; }

template<class T, class io> void data_seqv_file_base<T, io>::set_io_position(uint64_t i_pos)
{
   last_file_pos = read_position_v = write_position_v = i_pos; io()(file_v)->set_io_position(i_pos);
}

template<class T, class io> void data_seqv_file_base<T, io>::set_read_position(uint64_t i_pos)
{
   last_file_pos = read_position_v = i_pos; io()(file_v)->set_io_position(i_pos);
}

template<class T, class io> void data_seqv_file_base<T, io>::set_write_position(uint64_t i_pos)
{
   last_file_pos = write_position_v = i_pos; io()(file_v)->set_io_position(i_pos);
}

template<class T, class io> int data_seqv_file_base<T, io>::read_bytes_impl(std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   int bytes_read = 0;

   if (read_position() < size())
   {
      if (read_position() != last_file_pos) { set_read_position(read_position()); }
      bytes_read = io()(file_v)->read_bytes(i_seqv, i_elem_count, i_offset);
      last_file_pos = read_position();
   }

   if (bytes_read < 0 || static_cast<uint32_t>(bytes_read) != i_elem_count)
   {
      data_seqv_exception::throw_ex("reached end of file");
   }

   return bytes_read;
}

template<class T, class io> int data_seqv_file_base<T, io>::write_bytes_impl(const std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   if (write_position() != last_file_pos) { set_write_position(write_position()); }
   int bytes_written = io()(file_v)->write_bytes(i_seqv, i_elem_count, i_offset);
   last_file_pos = write_position();

   return bytes_written;
}


// data_seqv_reader_base
template<class T, class reader> data_seqv_reader_base<T, reader>& data_seqv_reader_base<T, reader>::operator=(data_seqv_reader_base<T, reader>&& i_seqv) noexcept
{
   if (this != &i_seqv)
   {
      seqv = std::move(i_seqv.seqv);
   }
   return *this;
}

template<class T, class reader> T& data_seqv_reader_base<T, reader>::dsv() { return seqv; }
template<class T, class reader> const T& data_seqv_reader_base<T, reader>::dsv() const { return seqv; }
template<class T, class reader> bool data_seqv_reader_base<T, reader>::is_end_of_seqv() { return reader()(seqv)->is_end_of_seqv(); }
template<class T, class reader> std::byte data_seqv_reader_base<T, reader>::read_byte() { return read<std::byte>(); }
template<class T, class reader> char data_seqv_reader_base<T, reader>::read_char() { return read<char>(); }
template<class T, class reader> int8_t data_seqv_reader_base<T, reader>::read_i8() { return read<int8_t>(); }
template<class T, class reader> uint8_t data_seqv_reader_base<T, reader>::read_u8() { return read<uint8_t>(); }
template<class T, class reader> int16_t data_seqv_reader_base<T, reader>::read_i16() { return read<int16_t>(); }
template<class T, class reader> uint16_t data_seqv_reader_base<T, reader>::read_u16() { return read<uint16_t>(); }
template<class T, class reader> int32_t data_seqv_reader_base<T, reader>::read_i32() { return read<int32_t>(); }
template<class T, class reader> uint32_t data_seqv_reader_base<T, reader>::read_u32() { return read<uint32_t>(); }
template<class T, class reader> int64_t data_seqv_reader_base<T, reader>::read_i64() { return read<int64_t>(); }
template<class T, class reader> uint64_t data_seqv_reader_base<T, reader>::read_u64() { return read<uint64_t>(); }
template<class T, class reader> float data_seqv_reader_base<T, reader>::read_f32() { return read<float>(); }
template<class T, class reader> double data_seqv_reader_base<T, reader>::read_f64() { return read<double>(); }

template<class T, class reader> template<class T0> T0 data_seqv_reader_base<T, reader>::read()
{
   union type_alias { T0 t; std::byte sq[sizeof(T0)]; } ta;
   read_bytes(ta.sq, sizeof(T0), 0);
   return ta.t;
}

template<class T, class reader> std::string data_seqv_reader_base<T, reader>::read_line()
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

template<class T, class reader> template<class T0> void data_seqv_reader_base<T, reader>::read_pointer(T0*& i_seqv)
{
   read_bytes(byte_cast(&i_seqv), sizeof(uintptr_t), 0);
}

template<class T, class reader> int data_seqv_reader_base<T, reader>::read_bytes(std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   return reader()(seqv)->read_bytes(i_seqv, i_elem_count, i_offset);
}

template<class T, class reader> std::vector<std::byte> data_seqv_reader_base<T, reader>::read_byte_vect(uint32_t i_elem_count)
{
   std::vector<std::byte> vect(i_elem_count);
   int bytes_read = read_bytes(vect.data(), i_elem_count, 0);
   assert(static_cast<uint32_t>(bytes_read) == i_elem_count);
   return vect;
}

template<class T, class reader> std::vector<std::byte> data_seqv_reader_base<T, reader>::read_sized_byte_vect()
{
   uint32_t size = read_u32();
   return read_byte_vect(size);
}

template<class T, class reader> std::string data_seqv_reader_base<T, reader>::read_sized_text()
{
   static_assert(sizeof(std::byte) == sizeof(char));
   uint32_t elem_count = read_u32();
   std::string text(elem_count, 0);
   read_bytes(byte_cast(text.data()), elem_count);

   return text;
}

template<class T, class reader> template<class T0> int data_seqv_reader_base<T, reader>::read(T0* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count * sizeof(T0), i_offset);
}

template<class T, class reader> int data_seqv_reader_base<T, reader>::read_chars(char* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count, i_offset);
}

template<class T, class reader> std::string data_seqv_reader_base<T, reader>::read_string(uint32_t i_elem_count)
{
   std::string text(i_elem_count, 0);
   [[maybe_unused]] int bytes_read = read_bytes(byte_cast(text.data()), i_elem_count);
   assert(bytes_read == i_elem_count);
   return text;
}

template<class T, class reader> int data_seqv_reader_base<T, reader>::read_i8(int8_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count, i_offset);
}

template<class T, class reader> int data_seqv_reader_base<T, reader>::read_u8(uint8_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count, i_offset);
}

template<class T, class reader> int data_seqv_reader_base<T, reader>::read_i16(int16_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count * 2, i_offset * 2);
}

template<class T, class reader> int data_seqv_reader_base<T, reader>::read_u16(uint16_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   return read_i16((int16_t*)i_seqv, i_elem_count, i_offset);
}

template<class T, class reader> int data_seqv_reader_base<T, reader>::read_i32(int32_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count * 4, i_offset * 4);
}

template<class T, class reader> int data_seqv_reader_base<T, reader>::read_u32(uint32_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   return read_i32((int32_t*)i_seqv, i_elem_count, i_offset);
}

template<class T, class reader> int data_seqv_reader_base<T, reader>::read_i64(int64_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   return read_bytes(byte_cast(i_seqv), i_elem_count * 8, i_offset * 8);
}

template<class T, class reader> int data_seqv_reader_base<T, reader>::read_u64(uint64_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   return read_i64((int64_t*)i_seqv, i_elem_count, i_offset);
}

template<class T, class reader> int data_seqv_reader_base<T, reader>::read_f32(float* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   return read_i32((int32_t*)i_seqv, i_elem_count, i_offset);
}

template<class T, class reader> int data_seqv_reader_base<T, reader>::read_f64(double* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   return read_i64((int64_t*)i_seqv, i_elem_count, i_offset);
}


// data_seqv_writer_base
template<class T, class writer> data_seqv_writer_base<T, writer>& data_seqv_writer_base<T, writer>::operator=(data_seqv_writer_base<T, writer>&& i_seqv) noexcept
{
   if (this != &i_seqv)
   {
      seqv = std::move(i_seqv.seqv);
   }
   return *this;
}

template<class T, class writer> T& data_seqv_writer_base<T, writer>::dsv() { return seqv; }
template<class T, class writer> const T& data_seqv_writer_base<T, writer>::dsv() const { return seqv; }
template<class T, class writer> void data_seqv_writer_base<T, writer>::write_byte(std::byte i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base<T, writer>::write_char(char i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base<T, writer>::write_i8(int8_t i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base<T, writer>::write_u8(uint8_t i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base<T, writer>::write_i16(int16_t i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base<T, writer>::write_u16(uint16_t i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base<T, writer>::write_i32(int32_t i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base<T, writer>::write_u32(uint32_t i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base<T, writer>::write_i64(int64_t i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base<T, writer>::write_u64(uint64_t i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base<T, writer>::write_f32(float i_seqv) { write(i_seqv); }
template<class T, class writer> void data_seqv_writer_base<T, writer>::write_f64(double i_seqv) { write(i_seqv); }
template<class T, class writer> template<class T0> void data_seqv_writer_base<T, writer>::write(const T0& i_data) { write_bytes(byte_const_cast(&i_data), sizeof(T0)); }

template<class T, class writer> void data_seqv_writer_base<T, writer>::write_line(const std::string& i_text, bool i_new_line)
{
   write_bytes(byte_const_cast(i_text.data()), i_text.length(), 0);
   if (i_new_line) { write<char>('\n'); }
}

template<class T, class writer> template<class T0> void data_seqv_writer_base<T, writer>::write_pointer(T0* const i_seqv)
{
   write_bytes(byte_const_cast(&i_seqv), sizeof(uintptr_t), 0);
}

template<class T, class writer> void data_seqv_writer_base<T, writer>::write_bytes(const std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   writer()(seqv)->write_bytes(i_seqv, i_elem_count, i_offset);
}

template<class T, class writer> void data_seqv_writer_base<T, writer>::write_sized_byte_vect(const std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   write_u32(i_elem_count);
   write_bytes(i_seqv, i_elem_count, i_offset);
}

template<class T, class writer> void data_seqv_writer_base<T, writer>::write_sized_text(const std::string& i_text)
{
   write_u32(i_text.length());
   write_bytes(byte_const_cast(i_text.data()), i_text.length(), 0);
}

template<class T, class writer> template<class T0> void data_seqv_writer_base<T, writer>::write(const T0* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   write_bytes(byte_const_cast(i_seqv), i_elem_count * sizeof(T0), i_offset * sizeof(T0));
}

template<class T, class writer> void data_seqv_writer_base<T, writer>::write_chars(const char* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   if (i_elem_count == 0) { i_elem_count = strlen(i_seqv); }
   write_bytes(byte_const_cast(i_seqv), i_elem_count, i_offset);
}

template<class T, class writer> void data_seqv_writer_base<T, writer>::write_string(const std::string& i_seqv)
{
   write_bytes(byte_const_cast(i_seqv.c_str()), i_seqv.length(), 0);
}

template<class T, class writer> void data_seqv_writer_base<T, writer>::write_i8(const int8_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   write_bytes(byte_const_cast(i_seqv), i_elem_count, i_offset);
}

template<class T, class writer> void data_seqv_writer_base<T, writer>::write_u8(const uint8_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   write_bytes(byte_const_cast(i_seqv), i_elem_count, i_offset);
}

template<class T, class writer> void data_seqv_writer_base<T, writer>::write_i16(const int16_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   write_bytes(byte_const_cast(i_seqv), i_elem_count * 2, i_offset * 2);
}

template<class T, class writer> void data_seqv_writer_base<T, writer>::write_u16(const uint16_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   write_i16((int16_t*)i_seqv, i_elem_count, i_offset);
}

template<class T, class writer> void data_seqv_writer_base<T, writer>::write_i32(const int32_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   write_bytes(byte_const_cast(i_seqv), i_elem_count * 4, i_offset * 4);
}

template<class T, class writer> void data_seqv_writer_base<T, writer>::write_u32(const uint32_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   write_i32((int32_t*)i_seqv, i_elem_count, i_offset);
}

template<class T, class writer> void data_seqv_writer_base<T, writer>::write_i64(const int64_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   write_bytes(byte_const_cast(i_seqv), i_elem_count * 8, i_offset * 8);
}

template<class T, class writer> void data_seqv_writer_base<T, writer>::write_u64(const uint64_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   write_i64((int64_t*)i_seqv, i_elem_count, i_offset);
}

template<class T, class writer> void data_seqv_writer_base<T, writer>::write_f32(const float* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   write_i32((int32_t*)i_seqv, i_elem_count, i_offset);
}

template<class T, class writer> void data_seqv_writer_base<T, writer>::write_f64(const double* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   write_i64((int64_t*)i_seqv, i_elem_count, i_offset);
}


// data_seqv_mem_writer
inline data_seqv_mem_writer& data_seqv_mem_writer::operator=(const data_seqv_mem_writer& i_seqv)
{
   if (this != &i_seqv)
   {
      seqv = i_seqv.seqv;
   }
   return *this;
}

inline data_seqv_mem_writer& data_seqv_mem_writer::operator=(data_seqv_mem_writer&& i_seqv) noexcept
{
   if (this != &i_seqv)
   {
      data_seqv_writer_base::operator=(std::move(i_seqv));
   }
   return *this;
}


// data_seqv_reader_big_endian
inline int8_t data_seqv_reader_big_endian::read_i8()
{
   int8_t sq;
   read_i8(&sq, 1, 0);

   return sq;
}

inline uint8_t data_seqv_reader_big_endian::read_u8()
{
   return (uint8_t)read_i8();
}

inline int16_t data_seqv_reader_big_endian::read_i16()
{
   int8_t sq[2];
   read_i8(sq, 2, 0);
   int16_t r = ((sq[1] & 0xff) | (sq[0] & 0xff) << 8);

   return r;
}

inline uint16_t data_seqv_reader_big_endian::read_u16()
{
   return (uint16_t)read_i16();
}

inline int32_t data_seqv_reader_big_endian::read_i32()
{
   int8_t sq[4];
   read_i8(sq, 4, 0);
   int32_t r = (sq[3] & 0xff) | ((sq[2] & 0xff) << 8) | ((sq[1] & 0xff) << 16) | ((sq[0] & 0xff) << 24);

   return r;
}

inline uint32_t data_seqv_reader_big_endian::read_u32()
{
   return (uint32_t)read_i32();
}

inline int64_t data_seqv_reader_big_endian::read_i64()
{
   int8_t sq[8];
   read_i8(sq, 8, 0);
   int64_t r = ((int64_t)(sq[7] & 0xff) | ((int64_t)(sq[6] & 0xff) << 8) | ((int64_t)(sq[5] & 0xff) << 16) | ((int64_t)(sq[4] & 0xff) << 24) |
      ((int64_t)(sq[3] & 0xff) << 32) | ((int64_t)(sq[2] & 0xff) << 40) | ((int64_t)(sq[1] & 0xff) << 48) | ((int64_t)(sq[0] & 0xff) << 56));

   return r;
}

inline uint64_t data_seqv_reader_big_endian::read_u64()
{
   return (uint64_t)read_i64();
}

inline float data_seqv_reader_big_endian::read_f32()
{
   int32_t r = read_i32();

   return *(float*)&r;
}

inline double data_seqv_reader_big_endian::read_f64()
{
   int64_t r = read_i64();

   return *(double*)&r;
}

inline void data_seqv_reader_big_endian::read_i8(int8_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   seqv->read_bytes(reinterpret_cast<std::byte*>(i_seqv), i_elem_count, i_offset);
}

inline void data_seqv_reader_big_endian::read_u8(uint8_t* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   read_i8((int8_t*)i_seqv, i_elem_count, i_offset);
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
