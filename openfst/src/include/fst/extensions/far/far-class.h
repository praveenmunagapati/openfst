// See www.openfst.org for extensive documentation on this weighted
// finite-state transducer library.
//
// Scripting API support for FarReader and FarWriter.

#ifndef FST_EXTENSIONS_FAR_FAR_CLASS_H_
#define FST_EXTENSIONS_FAR_FAR_CLASS_H_

#include <string>
#include <vector>

#include <fst/script/arg-packs.h>
#include <fst/script/fstscript.h>
#include <fst/extensions/far/far.h>


namespace fst {
namespace script {


// FarReader API.

// Virtual interface implemented by each concrete FarReaderImpl<Arc>.
// See the FarReader interface in far.h for the exact semantics.
class FarReaderImplBase {
 public:
  virtual const string &ArcType() const = 0;
  virtual bool Done() const = 0;
  virtual bool Error() const = 0;
  virtual const string &GetKey() const = 0;
  virtual const FstClass &GetFstClass() = 0;
  virtual bool Find(const string &key) = 0;
  virtual void Next() = 0;
  virtual void Reset() = 0;
  virtual FarType Type() const = 0;
  virtual ~FarReaderImplBase() {}
};


// Templated implementation.
template <class Arc>
class FarReaderClassImpl : public FarReaderImplBase {
 public:
  explicit FarReaderClassImpl(const string &filename) :
      impl_(FarReader<Arc>::Open(filename)) {}

  explicit FarReaderClassImpl(const std::vector<string> &filenames) :
      impl_(FarReader<Arc>::Open(filenames)) {}

  const string &ArcType() const override { return Arc::Type(); }

  bool Done() const override { return impl_->Done(); }

  bool Error() const override { return impl_->Error(); }

  bool Find(const string &key) override { return impl_->Find(key); }

  const FstClass &GetFstClass() override {
    fstc_ = FstClass(impl_->GetFst());
    return fstc_;
  }

  const string &GetKey() const override { return impl_->GetKey(); }

  void Next() override { return impl_->Next(); }

  void Reset() override { impl_->Reset(); }

  FarType Type() const override { return impl_->Type(); }

 private:
  std::unique_ptr<FarReader<Arc>> impl_;
  FstClass fstc_;
};


class FarReaderClass;

typedef args::WithReturnValue<FarReaderClass *, const string &>
    OpenFarReaderClassArgs1;

typedef args::WithReturnValue<FarReaderClass *, const std::vector<string> &>
    OpenFarReaderClassArgs2;


// Untemplated user-facing class holding a templated pimpl.
class FarReaderClass {
 public:
  const string &ArcType() const { return impl_->ArcType(); }

  bool Done() const { return impl_->Done(); }

  // Returns True if the impl is null (i.e., due to read failure).
  // Attempting to call any other function will result in null dereference.
  bool Error() const { return (impl_) ? impl_->Error() : true; }

  bool Find(const string &key) { return impl_->Find(key); }

  const FstClass &GetFstClass() { return impl_->GetFstClass(); }

  const string &GetKey() const { return impl_->GetKey(); }

  void Next() { impl_->Next(); }

  void Reset() { impl_->Reset(); }

  FarType Type() const { return impl_->Type(); }

  template <class Arc>
  friend void OpenFarReaderClass(OpenFarReaderClassArgs1 *args);

  template <class Arc>
  friend void OpenFarReaderClass(OpenFarReaderClassArgs2 *args);

  // Defined in the CC.

  static FarReaderClass *Open(const string &filename);

  static FarReaderClass *Open(const std::vector<string> &filenames);

 private:
  template <class Arc>
  explicit FarReaderClass(FarReaderClassImpl<Arc> *impl) : impl_(impl) {}

  std::unique_ptr<FarReaderImplBase> impl_;

  DISALLOW_COPY_AND_ASSIGN(FarReaderClass);
};

// These exist solely for registration purposes; users should call the
// static method FarReaderClass::Open instead.

template <class Arc>
void OpenFarReaderClass(OpenFarReaderClassArgs1 *args) {
  args->retval = new FarReaderClass(new FarReaderClassImpl<Arc>(args->args));
}

template <class Arc>
void OpenFarReaderClass(OpenFarReaderClassArgs2 *args) {
  args->retval = new FarReaderClass(new FarReaderClassImpl<Arc>(args->args));
}


// FarWriter API.

// Virtual interface implemented by each concrete FarWriterImpl<Arc>.
class FarWriterImplBase {
 public:
  // Unlike the library, this returns a boolean to signal failure due to
  // non-conformant arc types.
  virtual bool Add(const string &key, const FstClass &fst) = 0;
  virtual const string &ArcType() const = 0;
  virtual bool Error() const = 0;
  virtual FarType Type() const = 0;
  virtual ~FarWriterImplBase() {}
};


// Templated implementation.
template <class Arc>
class FarWriterClassImpl : public FarWriterImplBase {
 public:
  explicit FarWriterClassImpl(const string &filename,
                              FarType type = FAR_DEFAULT) :
      impl_(FarWriter<Arc>::Create(filename, type)) {}

  bool Add(const string &key, const FstClass &fst) override {
    if (ArcType() != fst.ArcType()) {
      FSTERROR() << "Cannot write FST with " << fst.ArcType() << " arcs to "
                 << "FAR with " << ArcType() << " arcs";
      return false;
    }
    impl_->Add(key, *(fst.GetFst<Arc>()));
    return true;
  }

  const string &ArcType() const override { return Arc::Type(); }

  bool Error() const override { return impl_->Error(); }

  FarType Type() const override { return impl_->Type(); }

 private:
  std::unique_ptr<FarWriter<Arc>> impl_;
};


class FarWriterClass;

typedef args::Package<const string &, FarType> CreateFarWriterClassInnerArgs;

typedef args::WithReturnValue<FarWriterClass *, CreateFarWriterClassInnerArgs>
    CreateFarWriterClassArgs;


// Untemplated user-facing class holding a templated pimpl.
class FarWriterClass {
 public:
  bool Add(const string &key, const FstClass &fst) {
    return impl_->Add(key, fst);
  }

  // Returns True if the impl is null (i.e., due to construction failure).
  // Attempting to call any other function will result in null dereference.
  bool Error() const { return (impl_) ? impl_->Error() : true; }

  const string &ArcType() const { return impl_->ArcType(); }

  FarType Type() const { return impl_->Type(); }

  template <class Arc>
  friend void CreateFarWriterClass(CreateFarWriterClassArgs *args);

  // Defined in the CC.
  static FarWriterClass *Create(const string &filename, const string &arc_type,
                                FarType type = FAR_DEFAULT);

 private:
  template <class Arc>
  explicit FarWriterClass(FarWriterClassImpl<Arc> *impl) : impl_(impl) {}

  std::unique_ptr<FarWriterImplBase> impl_;

  DISALLOW_COPY_AND_ASSIGN(FarWriterClass);
};

// This exists solely for registration purposes; users should call the
// static method FarWriterClass::Create instead.
template <class Arc>
void CreateFarWriterClass(CreateFarWriterClassArgs *args) {
  args->retval = new FarWriterClass(
      new FarWriterClassImpl<Arc>(args->args.arg1, args->args.arg2));
}

}  // namespace script
}  // namespace fst

#endif  // FST_EXTENSIONS_FAR_FAR_CLASS_H_