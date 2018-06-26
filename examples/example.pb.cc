// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: example.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "example.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace example {

namespace {

const ::google::protobuf::Descriptor* Ping_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  Ping_reflection_ = NULL;
const ::google::protobuf::Descriptor* PingRes_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  PingRes_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_example_2eproto() {
  protobuf_AddDesc_example_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "example.proto");
  GOOGLE_CHECK(file != NULL);
  Ping_descriptor_ = file->message_type(0);
  static const int Ping_offsets_[2] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Ping, address_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Ping, port_),
  };
  Ping_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      Ping_descriptor_,
      Ping::default_instance_,
      Ping_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Ping, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Ping, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(Ping));
  PingRes_descriptor_ = file->message_type(1);
  static const int PingRes_offsets_[2] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PingRes, res_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PingRes, mess_),
  };
  PingRes_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      PingRes_descriptor_,
      PingRes::default_instance_,
      PingRes_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PingRes, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PingRes, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(PingRes));
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_example_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    Ping_descriptor_, &Ping::default_instance());
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    PingRes_descriptor_, &PingRes::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_example_2eproto() {
  delete Ping::default_instance_;
  delete Ping_reflection_;
  delete PingRes::default_instance_;
  delete PingRes_reflection_;
}

void protobuf_AddDesc_example_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\rexample.proto\022\007example\"%\n\004Ping\022\017\n\007addr"
    "ess\030\002 \002(\t\022\014\n\004port\030\003 \002(\005\"$\n\007PingRes\022\013\n\003re"
    "s\030\001 \002(\005\022\014\n\004mess\030\002 \002(\t", 101);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "example.proto", &protobuf_RegisterTypes);
  Ping::default_instance_ = new Ping();
  PingRes::default_instance_ = new PingRes();
  Ping::default_instance_->InitAsDefaultInstance();
  PingRes::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_example_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_example_2eproto {
  StaticDescriptorInitializer_example_2eproto() {
    protobuf_AddDesc_example_2eproto();
  }
} static_descriptor_initializer_example_2eproto_;

// ===================================================================

#ifndef _MSC_VER
const int Ping::kAddressFieldNumber;
const int Ping::kPortFieldNumber;
#endif  // !_MSC_VER

Ping::Ping()
  : ::google::protobuf::Message() {
  SharedCtor();
}

void Ping::InitAsDefaultInstance() {
}

Ping::Ping(const Ping& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
}

void Ping::SharedCtor() {
  _cached_size_ = 0;
  address_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  port_ = 0;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

Ping::~Ping() {
  SharedDtor();
}

void Ping::SharedDtor() {
  if (address_ != &::google::protobuf::internal::kEmptyString) {
    delete address_;
  }
  if (this != default_instance_) {
  }
}

void Ping::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* Ping::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return Ping_descriptor_;
}

const Ping& Ping::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_example_2eproto();
  return *default_instance_;
}

Ping* Ping::default_instance_ = NULL;

Ping* Ping::New() const {
  return new Ping;
}

void Ping::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (has_address()) {
      if (address_ != &::google::protobuf::internal::kEmptyString) {
        address_->clear();
      }
    }
    port_ = 0;
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool Ping::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required string address = 2;
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_address()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->address().data(), this->address().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(24)) goto parse_port;
        break;
      }

      // required int32 port = 3;
      case 3: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_port:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &port_)));
          set_has_port();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }

      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void Ping::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // required string address = 2;
  if (has_address()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->address().data(), this->address().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      2, this->address(), output);
  }

  // required int32 port = 3;
  if (has_port()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(3, this->port(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* Ping::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // required string address = 2;
  if (has_address()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->address().data(), this->address().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        2, this->address(), target);
  }

  // required int32 port = 3;
  if (has_port()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(3, this->port(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int Ping::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required string address = 2;
    if (has_address()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->address());
    }

    // required int32 port = 3;
    if (has_port()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->port());
    }

  }
  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void Ping::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const Ping* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const Ping*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void Ping::MergeFrom(const Ping& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_address()) {
      set_address(from.address());
    }
    if (from.has_port()) {
      set_port(from.port());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void Ping::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void Ping::CopyFrom(const Ping& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Ping::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000003) != 0x00000003) return false;

  return true;
}

void Ping::Swap(Ping* other) {
  if (other != this) {
    std::swap(address_, other->address_);
    std::swap(port_, other->port_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata Ping::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = Ping_descriptor_;
  metadata.reflection = Ping_reflection_;
  return metadata;
}


// ===================================================================

#ifndef _MSC_VER
const int PingRes::kResFieldNumber;
const int PingRes::kMessFieldNumber;
#endif  // !_MSC_VER

PingRes::PingRes()
  : ::google::protobuf::Message() {
  SharedCtor();
}

void PingRes::InitAsDefaultInstance() {
}

PingRes::PingRes(const PingRes& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
}

void PingRes::SharedCtor() {
  _cached_size_ = 0;
  res_ = 0;
  mess_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

PingRes::~PingRes() {
  SharedDtor();
}

void PingRes::SharedDtor() {
  if (mess_ != &::google::protobuf::internal::kEmptyString) {
    delete mess_;
  }
  if (this != default_instance_) {
  }
}

void PingRes::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* PingRes::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return PingRes_descriptor_;
}

const PingRes& PingRes::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_example_2eproto();
  return *default_instance_;
}

PingRes* PingRes::default_instance_ = NULL;

PingRes* PingRes::New() const {
  return new PingRes;
}

void PingRes::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    res_ = 0;
    if (has_mess()) {
      if (mess_ != &::google::protobuf::internal::kEmptyString) {
        mess_->clear();
      }
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool PingRes::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required int32 res = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &res_)));
          set_has_res();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(18)) goto parse_mess;
        break;
      }

      // required string mess = 2;
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_mess:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_mess()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->mess().data(), this->mess().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }

      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void PingRes::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // required int32 res = 1;
  if (has_res()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(1, this->res(), output);
  }

  // required string mess = 2;
  if (has_mess()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->mess().data(), this->mess().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      2, this->mess(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* PingRes::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // required int32 res = 1;
  if (has_res()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(1, this->res(), target);
  }

  // required string mess = 2;
  if (has_mess()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->mess().data(), this->mess().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        2, this->mess(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int PingRes::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required int32 res = 1;
    if (has_res()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->res());
    }

    // required string mess = 2;
    if (has_mess()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->mess());
    }

  }
  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void PingRes::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const PingRes* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const PingRes*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void PingRes::MergeFrom(const PingRes& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_res()) {
      set_res(from.res());
    }
    if (from.has_mess()) {
      set_mess(from.mess());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void PingRes::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void PingRes::CopyFrom(const PingRes& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool PingRes::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000003) != 0x00000003) return false;

  return true;
}

void PingRes::Swap(PingRes* other) {
  if (other != this) {
    std::swap(res_, other->res_);
    std::swap(mess_, other->mess_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata PingRes::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = PingRes_descriptor_;
  metadata.reflection = PingRes_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace example

// @@protoc_insertion_point(global_scope)
