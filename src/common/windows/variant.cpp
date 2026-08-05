#include "variant.hpp"

#include <oleauto.h>

FFWmiVariant::FFWmiVariant(std::initializer_list<PCWSTR> strings) : FFWmiVariant() {
    SAFEARRAYBOUND bound = {
        .cElements = (ULONG) strings.size(),
        .lLbound = 0,
    };
    SAFEARRAY* psa = SafeArrayCreate(VT_BSTR, 1, &bound);

    LONG i = 0;
    for (PCWSTR str : strings) {
        SafeArrayPutElement(psa, &i, bstr_t(str));
        ++i;
    }

    this->vt = VT_ARRAY | VT_BSTR;
    this->parray = psa;
}
