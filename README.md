# data-sequence-bytebuffer-datastream

c++ header-only library for primitive type serialization similar(conceptually) to java's bytebuffer and datastream classes.
It can read/write data in memory/file sequences(streams) and can be extended for other streams as well.

No external dependencies.

# How to use

```c++
// write an int and a float to a memory sequence and then read them back
rw_seqv sequence;
sequence.w.write_i32(42);
sequence.w.write_f32(666.66f);
sequence.rewind();
int32_t meaning_of_life = sequence.r.read_i32();
float how_much_for_an_apple = sequence.r.read_f32();
sequence.rewind();

// write an int to a memory sequence and then read it back as a float
sequence.w.write_u32(0x5f3759df);
sequence.rewind();
float quakes_magic_number = sequence.r.read_f32();

// same steps, but with a file sequence now
rw_file_seqv sequence(std_file_wrapper("test.bin", "rwb"));
sequence.w.write_i32(42);
sequence.w.write_f32(666.66f);
sequence.rewind();
int32_t meaning_of_life = sequence.r.read_i32();
float how_much_for_an_apple = sequence.r.read_f32();
sequence.rewind();
sequence.w.write_u32(0x5f3759df);
sequence.rewind();
float quakes_magic_number = sequence.r.read_f32();

// sequence readers and writers also work with pointers to data sequences
mem_data_seqv* sequence_ptr = new mem_data_seqv();
data_seqv_writer_ptr dsw_ptr(sequence_ptr);
data_seqv_reader_ptr dsr_ptr(sequence_ptr);
dsw_ptr.write_u32(0x5f3759df);
dsr_ptr.data_sequence()->rewind(); // equivalent to sequence_ptr->rewind();
float quakes_magic_number = dsr_ptr.read_f32();
delete sequence_ptr;

// and shared pointers
std::shared_ptr<mem_data_seqv> sequence_sp = std::make_shared<mem_data_seqv>();
data_seqv_writer_sp dsw_sp(sequence_sp);
data_seqv_reader_sp dsr_sp(sequence_sp);
dsw_sp.write_u32(0x5f3759df);
dsr_sp.data_sequence()->rewind(); // equivalent to sequence_sp->rewind();
float quakes_magic_number = dsr_sp.read_f32();

// same steps also work for a file sequence by replacing mem_data_seqv with
// file_data_seqv( "new file_data_seqv(..);" or "std::make_shared<file_data_seqv>(..);" )
```
