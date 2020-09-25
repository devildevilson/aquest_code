#include "cryptopp/cryptlib.h"

#include "cryptopp/eccrypto.h"
#include "cryptopp/ec2n.h"
#include "cryptopp/luc.h"
#include "cryptopp/esign.h"
#include "cryptopp/rabin.h"
#include "cryptopp/gfpcrypt.h"
#include "cryptopp/modexppc.h"
#include "cryptopp/xtrcrypt.h"
#include "cryptopp/nbtheory.h"
#include "cryptopp/rsa.h"
#include "cryptopp/rw.h"

class fake_type_info : public std::type_info {
public:
  inline fake_type_info(const char* name) : std::type_info(name) {}
  inline virtual ~fake_type_info() {}
  //operator std::type_info () { return *this; }
};

template<> const std::type_info& fake_rtti<int> () { static fake_type_info type("int"); return type; }
template<> const std::type_info& fake_rtti<char const*> () { static fake_type_info type("char const*"); return type; }
template<> const std::type_info& fake_rtti<std::ostream*> () { static fake_type_info type("std::ostream*"); return type; }
template<> const std::type_info& fake_rtti<int const*> () { static fake_type_info type("int const*"); return type; }
template<> const std::type_info& fake_rtti<unsigned char> () { static fake_type_info type("unsigned char"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::ConstByteArrayParameter> () { static fake_type_info type("CryptoPP::ConstByteArrayParameter"); return type; }
template<> const std::type_info& fake_rtti<unsigned char const*> () { static fake_type_info type("unsigned char const*"); return type; }
template<> const std::type_info& fake_rtti<unsigned char*> () { static fake_type_info type("unsigned char*"); return type; }
template<> const std::type_info& fake_rtti<std::string*> () { static fake_type_info type("std::string*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters_IntegerBasedImpl<CryptoPP::ModExpPrecomputation, CryptoPP::DL_FixedBasePrecomputationImpl<CryptoPP::Integer> > > () { static fake_type_info type("CryptoPP::DL_GroupParameters_IntegerBasedImpl<CryptoPP::ModExpPrecomputation, CryptoPP::DL_FixedBasePrecomputationImpl<CryptoPP::Integer> > "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_DSA> > () { static fake_type_info type("CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_DSA> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKey<CryptoPP::Integer> > () { static fake_type_info type("CryptoPP::DL_PrivateKey<CryptoPP::Integer> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKey<CryptoPP::Integer>*> () { static fake_type_info type("CryptoPP::DL_PrivateKey<CryptoPP::Integer>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKey<CryptoPP::Integer> > () { static fake_type_info type("CryptoPP::DL_PublicKey<CryptoPP::Integer> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::Integer> () { static fake_type_info type("CryptoPP::Integer"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_DSA> > () { static fake_type_info type("CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_DSA> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters_DSA> () { static fake_type_info type("CryptoPP::DL_GroupParameters_DSA"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::OID> () { static fake_type_info type("CryptoPP::OID"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::ECP> () { static fake_type_info type("CryptoPP::ECP"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::ECPPoint> () { static fake_type_info type("CryptoPP::ECPPoint"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::EC2N> () { static fake_type_info type("CryptoPP::EC2N"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::EC2NPoint> () { static fake_type_info type("CryptoPP::EC2NPoint"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> > > () { static fake_type_info type("CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> > "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKey<CryptoPP::ECPPoint> > () { static fake_type_info type("CryptoPP::DL_PrivateKey<CryptoPP::ECPPoint> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKey<CryptoPP::ECPPoint>*> () { static fake_type_info type("CryptoPP::DL_PrivateKey<CryptoPP::ECPPoint>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKey<CryptoPP::ECPPoint> > () { static fake_type_info type("CryptoPP::DL_PublicKey<CryptoPP::ECPPoint> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_EC<CryptoPP::EC2N> > > () { static fake_type_info type("CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_EC<CryptoPP::EC2N> > "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKey<CryptoPP::EC2NPoint> > () { static fake_type_info type("CryptoPP::DL_PrivateKey<CryptoPP::EC2NPoint> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKey<CryptoPP::EC2NPoint>*> () { static fake_type_info type("CryptoPP::DL_PrivateKey<CryptoPP::EC2NPoint>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKey<CryptoPP::EC2NPoint> > () { static fake_type_info type("CryptoPP::DL_PublicKey<CryptoPP::EC2NPoint> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> > > () { static fake_type_info type("CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> > "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> > () { static fake_type_info type("CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_EC<CryptoPP::EC2N> > > () { static fake_type_info type("CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_EC<CryptoPP::EC2N> > "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters_EC<CryptoPP::EC2N> > () { static fake_type_info type("CryptoPP::DL_GroupParameters_EC<CryptoPP::EC2N> "); return type; }
template<> const std::type_info& fake_rtti<std::string> () { static fake_type_info type("std::string"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters_GFP> () { static fake_type_info type("CryptoPP::DL_GroupParameters_GFP"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters_IntegerBased> () { static fake_type_info type("CryptoPP::DL_GroupParameters_IntegerBased"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters_GFP*> () { static fake_type_info type("CryptoPP::DL_GroupParameters_GFP*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters<CryptoPP::Integer> > () { static fake_type_info type("CryptoPP::DL_GroupParameters<CryptoPP::Integer> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters<CryptoPP::Integer>*> () { static fake_type_info type("CryptoPP::DL_GroupParameters<CryptoPP::Integer>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters_IntegerBasedImpl<CryptoPP::ModExpPrecomputation, CryptoPP::DL_FixedBasePrecomputationImpl<CryptoPP::Integer> >*> () { static fake_type_info type("CryptoPP::DL_GroupParameters_IntegerBasedImpl<CryptoPP::ModExpPrecomputation, CryptoPP::DL_FixedBasePrecomputationImpl<CryptoPP::Integer> >*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_DSA>*> () { static fake_type_info type("CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_DSA>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKey<CryptoPP::Integer>*> () { static fake_type_info type("CryptoPP::DL_PublicKey<CryptoPP::Integer>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_DSA>*> () { static fake_type_info type("CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_DSA>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters<CryptoPP::ECPPoint> > () { static fake_type_info type("CryptoPP::DL_GroupParameters<CryptoPP::ECPPoint> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters<CryptoPP::ECPPoint>*> () { static fake_type_info type("CryptoPP::DL_GroupParameters<CryptoPP::ECPPoint>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP>*> () { static fake_type_info type("CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters<CryptoPP::EC2NPoint> > () { static fake_type_info type("CryptoPP::DL_GroupParameters<CryptoPP::EC2NPoint> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters<CryptoPP::EC2NPoint>*> () { static fake_type_info type("CryptoPP::DL_GroupParameters<CryptoPP::EC2NPoint>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters_EC<CryptoPP::EC2N>*> () { static fake_type_info type("CryptoPP::DL_GroupParameters_EC<CryptoPP::EC2N>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> >*> () { static fake_type_info type("CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> >*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKey<CryptoPP::ECPPoint>*> () { static fake_type_info type("CryptoPP::DL_PublicKey<CryptoPP::ECPPoint>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_EC<CryptoPP::EC2N> >*> () { static fake_type_info type("CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_EC<CryptoPP::EC2N> >*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKey<CryptoPP::EC2NPoint>*> () { static fake_type_info type("CryptoPP::DL_PublicKey<CryptoPP::EC2NPoint>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> >*> () { static fake_type_info type("CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> >*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_EC<CryptoPP::EC2N> >*> () { static fake_type_info type("CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_EC<CryptoPP::EC2N> >*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::ESIGNFunction> () { static fake_type_info type("CryptoPP::ESIGNFunction"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::InvertibleESIGNFunction> () { static fake_type_info type("CryptoPP::InvertibleESIGNFunction"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::ESIGNFunction*> () { static fake_type_info type("CryptoPP::ESIGNFunction*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::InvertibleESIGNFunction*> () { static fake_type_info type("CryptoPP::InvertibleESIGNFunction*"); return type; }
template<> const std::type_info& fake_rtti<wchar_t const*> () { static fake_type_info type("wchar_t const*"); return type; }
template<> const std::type_info& fake_rtti<std::istream*> () { static fake_type_info type("std::istream*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::ByteArrayParameter> () { static fake_type_info type("CryptoPP::ByteArrayParameter"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::BufferedTransformation*> () { static fake_type_info type("CryptoPP::BufferedTransformation*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::RandomNumberGenerator*> () { static fake_type_info type("CryptoPP::RandomNumberGenerator*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::BlockPaddingSchemeDef::BlockPaddingScheme> () { static fake_type_info type("CryptoPP::BlockPaddingSchemeDef::BlockPaddingScheme"); return type; }
template<> const std::type_info& fake_rtti<bool> () { static fake_type_info type("bool"); return type; }
template<> const std::type_info& fake_rtti<unsigned int> () { static fake_type_info type("unsigned int"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters_IntegerBased*> () { static fake_type_info type("CryptoPP::DL_GroupParameters_IntegerBased*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::Integer::RandomNumberType> () { static fake_type_info type("CryptoPP::Integer::RandomNumberType"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::PrimeSelector const*> () { static fake_type_info type("CryptoPP::PrimeSelector const*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::LUCFunction> () { static fake_type_info type("CryptoPP::LUCFunction"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::InvertibleLUCFunction> () { static fake_type_info type("CryptoPP::InvertibleLUCFunction"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::LUCFunction*> () { static fake_type_info type("CryptoPP::LUCFunction*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::InvertibleLUCFunction*> () { static fake_type_info type("CryptoPP::InvertibleLUCFunction*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::RabinFunction> () { static fake_type_info type("CryptoPP::RabinFunction"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::InvertibleRabinFunction> () { static fake_type_info type("CryptoPP::InvertibleRabinFunction"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::RabinFunction*> () { static fake_type_info type("CryptoPP::RabinFunction*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::InvertibleRabinFunction*> () { static fake_type_info type("CryptoPP::InvertibleRabinFunction*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_GFP_DefaultSafePrime> > () { static fake_type_info type("CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_GFP_DefaultSafePrime> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_GFP_DefaultSafePrime> > () { static fake_type_info type("CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_GFP_DefaultSafePrime> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters_GFP_DefaultSafePrime> () { static fake_type_info type("CryptoPP::DL_GroupParameters_GFP_DefaultSafePrime"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_GFP> > () { static fake_type_info type("CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_GFP> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_GFP> > () { static fake_type_info type("CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_GFP> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_GFP_DefaultSafePrime>*> () { static fake_type_info type("CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_GFP_DefaultSafePrime>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_GFP_DefaultSafePrime>*> () { static fake_type_info type("CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_GFP_DefaultSafePrime>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_GFP>*> () { static fake_type_info type("CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_GFP>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_GFP>*> () { static fake_type_info type("CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_GFP>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::RSAFunction> () { static fake_type_info type("CryptoPP::RSAFunction"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::InvertibleRSAFunction> () { static fake_type_info type("CryptoPP::InvertibleRSAFunction"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::RSAFunction*> () { static fake_type_info type("CryptoPP::RSAFunction*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::InvertibleRSAFunction*> () { static fake_type_info type("CryptoPP::InvertibleRSAFunction*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::RWFunction> () { static fake_type_info type("CryptoPP::RWFunction"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::InvertibleRWFunction> () { static fake_type_info type("CryptoPP::InvertibleRWFunction"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::RWFunction*> () { static fake_type_info type("CryptoPP::RWFunction*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::InvertibleRWFunction*> () { static fake_type_info type("CryptoPP::InvertibleRWFunction*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters_IntegerBasedImpl<CryptoPP::DL_GroupPrecomputation_LUC, CryptoPP::DL_BasePrecomputation_LUC> > () { static fake_type_info type("CryptoPP::DL_GroupParameters_IntegerBasedImpl<CryptoPP::DL_GroupPrecomputation_LUC, CryptoPP::DL_BasePrecomputation_LUC> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_LUC> > () { static fake_type_info type("CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_LUC> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_LUC> > () { static fake_type_info type("CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_LUC> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters_LUC> () { static fake_type_info type("CryptoPP::DL_GroupParameters_LUC"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_LUC_DefaultSafePrime> > () { static fake_type_info type("CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_LUC_DefaultSafePrime> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_LUC_DefaultSafePrime> > () { static fake_type_info type("CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_LUC_DefaultSafePrime> "); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters_LUC_DefaultSafePrime> () { static fake_type_info type("CryptoPP::DL_GroupParameters_LUC_DefaultSafePrime"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters_LUC*> () { static fake_type_info type("CryptoPP::DL_GroupParameters_LUC*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_GroupParameters_IntegerBasedImpl<CryptoPP::DL_GroupPrecomputation_LUC, CryptoPP::DL_BasePrecomputation_LUC>*> () { static fake_type_info type("CryptoPP::DL_GroupParameters_IntegerBasedImpl<CryptoPP::DL_GroupPrecomputation_LUC, CryptoPP::DL_BasePrecomputation_LUC>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_LUC>*> () { static fake_type_info type("CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_LUC>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_LUC>*> () { static fake_type_info type("CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_LUC>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_LUC_DefaultSafePrime>*> () { static fake_type_info type("CryptoPP::DL_PublicKeyImpl<CryptoPP::DL_GroupParameters_LUC_DefaultSafePrime>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_LUC_DefaultSafePrime>*> () { static fake_type_info type("CryptoPP::DL_PrivateKeyImpl<CryptoPP::DL_GroupParameters_LUC_DefaultSafePrime>*"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::XTR_DH> () { static fake_type_info type("CryptoPP::XTR_DH"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::GFP2Element> () { static fake_type_info type("CryptoPP::GFP2Element"); return type; }
template<> const std::type_info& fake_rtti<CryptoPP::XTR_DH*> () { static fake_type_info type("CryptoPP::XTR_DH*"); return type; }
