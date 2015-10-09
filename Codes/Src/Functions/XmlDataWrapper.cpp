#include "SystemInclude.h"
#include "Common.h"
#include "SystemError.h" 

#include "Types.h"
#include "Descriptor.h"
#include "Nit.h"
#include "Sdt.h"
#include "Bat.h"
#include "Eit.h"
#include "XmlDataWrapperInl.h"

/* instanciate class NitXmlWrapper for Nit.
*/
template NitXmlWrapper<Nit>;
template SdtXmlWrapper<Sdt>;
template BatXmlWrapper<Bat>;
template EitXmlWrapper<Eit>;

