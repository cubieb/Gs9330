#include "SystemInclude.h"
#include "Common.h"
#include "SystemError.h" 

#include "Types.h"
#include "Descriptor.h"
#include "Nit.h"
#include "Sdt.h"
#include "Bat.h"
#include "Eit.h"
#include "TsDataWrapperInl.h"

/* instanciate class NitTsWrapper for Nit.
*/
template NitTsWrapper<Nit>;
template SdtTsWrapper<Sdt>;
