# data-sequence

c++ header-only library for primitive type serialization similar(conceptually) to java's bytebuffer and datastream classes.
It can read/write data in memory/file sequences(streams) and can be extended for other streams as well.

No external dependencies.

# How to use

There are only two concepts used:
- the first one is the data sequence type, which contains the data, be it in a file, in memory, etc.
- and the second one is the data handler, i.e. readers and writers, which are applied to the data sequences.

```c++
  #include <data-seqv.hxx>

  {
	 // write an int and a float to a memory sequence and then read them back
	 data_seqv_rw_mem_ops sequence;
	 sequence.w.write_i32(42);
	 sequence.w.write_f32(666.66f);
	 sequence.rewind();
	 int32_t meaning_of_life = sequence.r.read_i32();
	 assert(meaning_of_life == 42);
	 float how_much_for_an_apple = sequence.r.read_f32();
	 assert(how_much_for_an_apple == 666.66f);
	 sequence.rewind();

	 // write an int to a memory sequence and then read it back as a float
	 sequence.w.write_u32(0x5f3759df);
	 sequence.rewind();
	 float magic_number = sequence.r.read_f32();
  }
  {
	 // same steps, but with a file sequence now
	 // and also showing the equivalent template versions for reading/writing
	 data_seqv_rw_file_ops sequence(data_seqv_std_file_wrapper(file_path, "w+b"));
	 sequence.w.write(42);
	 sequence.w.write(666.66f);
	 sequence.rewind();
	 int32_t meaning_of_life = sequence.r.read<int>();
	 assert(meaning_of_life == 42);
	 float how_much_for_an_apple = sequence.r.read<float>();
	 assert(how_much_for_an_apple == 666.66f);
	 sequence.rewind();
	 sequence.w.write(0x5f3759df);
	 sequence.rewind();
	 float magic_number = sequence.r.read_f32();
  }
  {
	 // sequence readers and writers also work with pointers to data sequences
	 data_seqv_rw_mem* sequence_ptr = new data_seqv_rw_mem();
	 data_seqv_writer_ptr dsw_ptr(sequence_ptr);
	 data_seqv_reader_ptr dsr_ptr(sequence_ptr);
	 dsw_ptr.write_u32(0x5f3759df);
	 dsr_ptr.dsv()->rewind(); // equivalent to sequence_ptr->rewind();
	 float magic_number = dsr_ptr.read_f32();
	 delete sequence_ptr;
  }
  {
	 // and shared pointers
	 std::shared_ptr<data_seqv_rw_mem> sequence_sp = std::make_shared<data_seqv_rw_mem>();
	 data_seqv_writer_shr dsw_sp(sequence_sp);
	 data_seqv_reader_shr dsr_sp(sequence_sp);
	 dsw_sp.write_u32(0x5f3759df);
	 dsr_sp.dsv()->rewind(); // equivalent to sequence_sp->rewind();
	 float magic_number = dsr_sp.read_f32();
  }
  // same steps also work for a file sequence by replacing data_seqv_rw_mem with
  // data_seqv_file( "new data_seqv_file(..);" or "std::make_shared<data_seqv_file>(..);" )
  {
	 const uint32_t nr = 0x12345678;
	 data_seqv_rw_mem_ops seqv;
	 seqv.w.write_u32(nr);
	 seqv.w.write(nr);
	 seqv.rewind();
	 uint32_t t0 = seqv.r.read_u32();
	 uint32_t t1 = seqv.r.read<uint32_t>();
	 seqv.rewind();
	 uint32_t t2 = seqv.r.read<uint32_t>();
	 uint32_t t3 = seqv.r.read_u32();
	 assert(t0 == nr);
	 assert(t0 == t1);
	 assert(t0 == t2);
	 assert(t0 == t3);
	 seqv.rewind();
	 seqv.w.write_u32(nr);
	 seqv.rewind();
	 float f0 = seqv.r.read_f32();
	 seqv.rewind();
	 seqv.w.write_f32(f0);
	 seqv.rewind();
	 uint32_t t4 = seqv.r.read_u32();
	 assert(t0 == t4);
  }
```
