#include "chunk.hpp"
#include "fmt/format.h"

using namespace fmt::literals;

Chunk::Chunk(TypePtr type, Context *context) : type(type), context(context) {}

TypePtr Chunk::get_type() const { return type; }

int Chunk::get_offset() const { return 0; }

void Chunk::copy_from(ChunkPtr other){
	//TODO: this only works for primitives
	auto my_reg = load();
	auto other_reg = other->load();
	*context->get_stream() << "\tmove\t${},${}\n"_format(my_reg, other_reg);
	other->discard();
	store();
}

LocalChunk::LocalChunk(TypePtr type, Context *context) : Chunk(type, context) {
  offset = context->get_stack_size();
}

int LocalChunk::get_offset() const { return offset; }

void LocalChunk::set_offset(const int& in_offset) { offset = in_offset; }

void LocalChunk::store() {
  context->regs[*reg] = true;
  *context->get_stream() << "\tsw\t${}, {}($fp)\n"_format(*reg, -get_offset());
                         //<< "\tnop\n";
  reg.reset();
}

unsigned LocalChunk::get_reg() const { return *reg; }

unsigned LocalChunk::load() {
  if (reg) {
    return *reg;
  }
  reg = context->allocate_reg();
  if (reg != -1) {
    *context->get_stream() << "\tlw\t${}, {}($fp)\n"_format(*reg, -get_offset())
                           << "\tnop\n";
    return *reg;
  } else {
    *context->get_stream() << "\t#fatal error, out of free registers\n";
    return 0;
  }
}

void LocalChunk::discard() {
  context->regs[*reg] = true;
  reg.reset();
}

GlobalChunk::GlobalChunk(TypePtr type, Context *context, std::string name)
    : Chunk(type, context), name(name) {}

void GlobalChunk::store() {
	unsigned tmp_reg = 25;
	if (tmp_reg != 1){
	*context->get_stream() << "\tlw\t${}, %got({})($28)\n"_format(tmp_reg, name)
						   << "\tnop\n"
						   << "\tsw\t${}, 0(${})\n"_format(*reg, tmp_reg) 
						   << "\tnop\n"; 
	}
	context->regs[*reg] = true;
	reg.reset();
}

unsigned GlobalChunk::get_reg() const { return *reg; }

unsigned GlobalChunk::load() {
	if (reg) {
		return *reg;
	}
	reg = context->allocate_reg();
	unsigned tmp_reg = 25;
	if (reg != -1 && tmp_reg != -1){
		*context->get_stream() << "\tlw\t${}, %got({})($28)\n"_format(tmp_reg, name)
							   << "\tnop\n"
							   << "\tlw\t${}, 0(${})\n"_format(*reg, tmp_reg)
							   << "\tnop\n";
		return *reg;
	} else {
		*context->get_stream() << "\t #fatal error, out of free registers\n"; 
		return 0;
	}
}

void GlobalChunk::discard() {
  context->regs[*reg] = true;
  reg.reset();
}
