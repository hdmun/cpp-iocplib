// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_CLIENTMESSAGE_MESSAGE_CLIENT_H_
#define FLATBUFFERS_GENERATED_CLIENTMESSAGE_MESSAGE_CLIENT_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 22 &&
              FLATBUFFERS_VERSION_MINOR == 10 &&
              FLATBUFFERS_VERSION_REVISION == 26,
             "Non-compatible flatbuffers version included");

namespace message {
namespace client {

struct Packet;
struct PacketBuilder;

struct LoginRequest;
struct LoginRequestBuilder;

struct ConnectServerRequest;
struct ConnectServerRequestBuilder;

struct LogoutRequest;
struct LogoutRequestBuilder;

enum PacketType : uint8_t {
  PacketType_NONE = 0,
  PacketType_LoginRequest = 1,
  PacketType_LogoutRequest = 2,
  PacketType_ConnectServerRequest = 3,
  PacketType_MIN = PacketType_NONE,
  PacketType_MAX = PacketType_ConnectServerRequest
};

inline const PacketType (&EnumValuesPacketType())[4] {
  static const PacketType values[] = {
    PacketType_NONE,
    PacketType_LoginRequest,
    PacketType_LogoutRequest,
    PacketType_ConnectServerRequest
  };
  return values;
}

inline const char * const *EnumNamesPacketType() {
  static const char * const names[5] = {
    "NONE",
    "LoginRequest",
    "LogoutRequest",
    "ConnectServerRequest",
    nullptr
  };
  return names;
}

inline const char *EnumNamePacketType(PacketType e) {
  if (flatbuffers::IsOutRange(e, PacketType_NONE, PacketType_ConnectServerRequest)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesPacketType()[index];
}

template<typename T> struct PacketTypeTraits {
  static const PacketType enum_value = PacketType_NONE;
};

template<> struct PacketTypeTraits<message::client::LoginRequest> {
  static const PacketType enum_value = PacketType_LoginRequest;
};

template<> struct PacketTypeTraits<message::client::LogoutRequest> {
  static const PacketType enum_value = PacketType_LogoutRequest;
};

template<> struct PacketTypeTraits<message::client::ConnectServerRequest> {
  static const PacketType enum_value = PacketType_ConnectServerRequest;
};

bool VerifyPacketType(flatbuffers::Verifier &verifier, const void *obj, PacketType type);
bool VerifyPacketTypeVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types);

struct Packet FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef PacketBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_TYPE_TYPE = 4,
    VT_TYPE = 6
  };
  message::client::PacketType type_type() const {
    return static_cast<message::client::PacketType>(GetField<uint8_t>(VT_TYPE_TYPE, 0));
  }
  const void *type() const {
    return GetPointer<const void *>(VT_TYPE);
  }
  template<typename T> const T *type_as() const;
  const message::client::LoginRequest *type_as_LoginRequest() const {
    return type_type() == message::client::PacketType_LoginRequest ? static_cast<const message::client::LoginRequest *>(type()) : nullptr;
  }
  const message::client::LogoutRequest *type_as_LogoutRequest() const {
    return type_type() == message::client::PacketType_LogoutRequest ? static_cast<const message::client::LogoutRequest *>(type()) : nullptr;
  }
  const message::client::ConnectServerRequest *type_as_ConnectServerRequest() const {
    return type_type() == message::client::PacketType_ConnectServerRequest ? static_cast<const message::client::ConnectServerRequest *>(type()) : nullptr;
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_TYPE_TYPE, 1) &&
           VerifyOffset(verifier, VT_TYPE) &&
           VerifyPacketType(verifier, type(), type_type()) &&
           verifier.EndTable();
  }
};

template<> inline const message::client::LoginRequest *Packet::type_as<message::client::LoginRequest>() const {
  return type_as_LoginRequest();
}

template<> inline const message::client::LogoutRequest *Packet::type_as<message::client::LogoutRequest>() const {
  return type_as_LogoutRequest();
}

template<> inline const message::client::ConnectServerRequest *Packet::type_as<message::client::ConnectServerRequest>() const {
  return type_as_ConnectServerRequest();
}

struct PacketBuilder {
  typedef Packet Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_type_type(message::client::PacketType type_type) {
    fbb_.AddElement<uint8_t>(Packet::VT_TYPE_TYPE, static_cast<uint8_t>(type_type), 0);
  }
  void add_type(flatbuffers::Offset<void> type) {
    fbb_.AddOffset(Packet::VT_TYPE, type);
  }
  explicit PacketBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Packet> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Packet>(end);
    return o;
  }
};

inline flatbuffers::Offset<Packet> CreatePacket(
    flatbuffers::FlatBufferBuilder &_fbb,
    message::client::PacketType type_type = message::client::PacketType_NONE,
    flatbuffers::Offset<void> type = 0) {
  PacketBuilder builder_(_fbb);
  builder_.add_type(type);
  builder_.add_type_type(type_type);
  return builder_.Finish();
}

struct LoginRequest FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef LoginRequestBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ID = 4,
    VT_PASSWORD = 6
  };
  const flatbuffers::String *id() const {
    return GetPointer<const flatbuffers::String *>(VT_ID);
  }
  const flatbuffers::String *password() const {
    return GetPointer<const flatbuffers::String *>(VT_PASSWORD);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_ID) &&
           verifier.VerifyString(id()) &&
           VerifyOffset(verifier, VT_PASSWORD) &&
           verifier.VerifyString(password()) &&
           verifier.EndTable();
  }
};

struct LoginRequestBuilder {
  typedef LoginRequest Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_id(flatbuffers::Offset<flatbuffers::String> id) {
    fbb_.AddOffset(LoginRequest::VT_ID, id);
  }
  void add_password(flatbuffers::Offset<flatbuffers::String> password) {
    fbb_.AddOffset(LoginRequest::VT_PASSWORD, password);
  }
  explicit LoginRequestBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<LoginRequest> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<LoginRequest>(end);
    return o;
  }
};

inline flatbuffers::Offset<LoginRequest> CreateLoginRequest(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> id = 0,
    flatbuffers::Offset<flatbuffers::String> password = 0) {
  LoginRequestBuilder builder_(_fbb);
  builder_.add_password(password);
  builder_.add_id(id);
  return builder_.Finish();
}

inline flatbuffers::Offset<LoginRequest> CreateLoginRequestDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *id = nullptr,
    const char *password = nullptr) {
  auto id__ = id ? _fbb.CreateString(id) : 0;
  auto password__ = password ? _fbb.CreateString(password) : 0;
  return message::client::CreateLoginRequest(
      _fbb,
      id__,
      password__);
}

struct ConnectServerRequest FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ConnectServerRequestBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SERVER_ID = 4
  };
  int16_t server_id() const {
    return GetField<int16_t>(VT_SERVER_ID, -1);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int16_t>(verifier, VT_SERVER_ID, 2) &&
           verifier.EndTable();
  }
};

struct ConnectServerRequestBuilder {
  typedef ConnectServerRequest Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_server_id(int16_t server_id) {
    fbb_.AddElement<int16_t>(ConnectServerRequest::VT_SERVER_ID, server_id, -1);
  }
  explicit ConnectServerRequestBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<ConnectServerRequest> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ConnectServerRequest>(end);
    return o;
  }
};

inline flatbuffers::Offset<ConnectServerRequest> CreateConnectServerRequest(
    flatbuffers::FlatBufferBuilder &_fbb,
    int16_t server_id = -1) {
  ConnectServerRequestBuilder builder_(_fbb);
  builder_.add_server_id(server_id);
  return builder_.Finish();
}

struct LogoutRequest FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef LogoutRequestBuilder Builder;
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           verifier.EndTable();
  }
};

struct LogoutRequestBuilder {
  typedef LogoutRequest Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  explicit LogoutRequestBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<LogoutRequest> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<LogoutRequest>(end);
    return o;
  }
};

inline flatbuffers::Offset<LogoutRequest> CreateLogoutRequest(
    flatbuffers::FlatBufferBuilder &_fbb) {
  LogoutRequestBuilder builder_(_fbb);
  return builder_.Finish();
}

inline bool VerifyPacketType(flatbuffers::Verifier &verifier, const void *obj, PacketType type) {
  switch (type) {
    case PacketType_NONE: {
      return true;
    }
    case PacketType_LoginRequest: {
      auto ptr = reinterpret_cast<const message::client::LoginRequest *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case PacketType_LogoutRequest: {
      auto ptr = reinterpret_cast<const message::client::LogoutRequest *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case PacketType_ConnectServerRequest: {
      auto ptr = reinterpret_cast<const message::client::ConnectServerRequest *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return true;
  }
}

inline bool VerifyPacketTypeVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyPacketType(
        verifier,  values->Get(i), types->GetEnum<PacketType>(i))) {
      return false;
    }
  }
  return true;
}

inline const message::client::Packet *GetPacket(const void *buf) {
  return flatbuffers::GetRoot<message::client::Packet>(buf);
}

inline const message::client::Packet *GetSizePrefixedPacket(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<message::client::Packet>(buf);
}

inline bool VerifyPacketBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<message::client::Packet>(nullptr);
}

inline bool VerifySizePrefixedPacketBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<message::client::Packet>(nullptr);
}

inline void FinishPacketBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<message::client::Packet> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedPacketBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<message::client::Packet> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace client
}  // namespace message

#endif  // FLATBUFFERS_GENERATED_CLIENTMESSAGE_MESSAGE_CLIENT_H_
