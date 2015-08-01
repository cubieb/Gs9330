#include "SystemInclude.h"
#include "Crc32.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function initializes "CRC Lookup Table". You only need to call it once to
		initalize the table before using any of the other CRC32 calculation functions.
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Crc32::Crc32(void)
{
    this->Initialize();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Crc32::~Crc32(void)
{
    //No destructor code.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function initializes "CRC Lookup Table". You only need to call it once to
		initalize the table before using any of the other CRC32 calculation functions.
*/

void Crc32::Initialize(void)
{
	//0x04C11DB7 is the official polynomial used by PKZip, WinZip and Ethernet.
	uint32_t polynomial = 0x04C11DB7;

	memset(&this->table, 0, sizeof(this->table));

	// 256 values representing ASCII character codes.
	for(int codes = 0; codes <= 0xFF; codes++)
	{
		this->table[codes] = this->Reflect(codes, 8) << 24;

		for(int pos = 0; pos < 8; pos++)
		{
			this->table[codes] = (this->table[codes] << 1)
				^ ((this->table[codes] & (1 << 31)) ? polynomial : 0);
		}

		this->table[codes] = this->Reflect(this->table[codes], 32);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Reflection is a requirement for the official CRC-32 standard.
	You can create CRCs without it, but they won't conform to the standard.
*/

uint32_t Crc32::Reflect(uint32_t reflect, const char ch)
{
	uint32_t value = 0;

	// Swap bit 0 for bit 7, bit 1 For bit 6, etc....
	for(int pos = 1; pos < (ch + 1); pos++)
	{
		if(reflect & 1)
		{
			value |= (1 << (ch - pos));
		}
		reflect >>= 1;
	}

	return value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Calculates the CRC32 by looping through each of the bytes in buffer.
	
	Note: For Example usage example, see FileCrc().
*/

void Crc32::PartialCrc(uint32_t *crc, const unsigned char *buffer, size_t iDataLength)
{
	while(iDataLength--)
	{
		//If your compiler complains about the following line, try changing
		//	each occurrence of *crc with ((uint32_t)*crc).

		*crc = (*crc >> 8) ^ this->table[(*crc & 0xFF) ^ *buffer++];
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the calculated CRC32 (through crc) for the given string.
*/

void Crc32::FullCrc(const unsigned char *buffer, size_t iDataLength, uint32_t *crc)
{
    ((uint32_t)*crc) = 0xffffffff; //Initilaize the CRC.

	this->PartialCrc(crc, buffer, iDataLength);

	((uint32_t)*crc) ^= 0xffffffff; //Finalize the CRC.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the calculated CRC23 for the given string.
*/

uint32_t Crc32::FullCrc(const unsigned char *buffer, size_t iDataLength)
{
    uint32_t crc = 0xffffffff; //Initilaize the CRC.

	this->PartialCrc(&crc, buffer, iDataLength);

	return(crc ^ 0xffffffff); //Finalize the CRC and return.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Calculates the CRC32 of a file using the a user defined buffer.

	Note: The buffer size DOES NOT affect the resulting CRC,
			it has been provided for performance purposes only.
*/

bool Crc32::FileCrc(const char *fileName, uint32_t *crc, size_t bufferSize)
{
    ((uint32_t)*crc) = 0xffffffff; //Initilaize the CRC.

	FILE *fSource = NULL;
	unsigned char *buf = NULL;
	size_t lenRead = 0;

	if((fSource = fopen(fileName, "rb")) == NULL)
	{
		return false; //Failed to open file for read access.
	}

    if(!(buf = (unsigned char *)malloc(bufferSize))) //Allocate memory for file buffering.
	{
		fclose(fSource);
		return false; //Out of memory.
	}

	while((lenRead = fread(buf, sizeof(char), bufferSize, fSource)))
	{
		this->PartialCrc(crc, buf, lenRead);
	}

    free(buf);
	fclose(fSource);

	((uint32_t)*crc) ^= 0xffffffff; //Finalize the CRC.

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Calculates the CRC32 of a file using the a default buffer size of 1MB.
*/

uint32_t Crc32::FileCrc(const char *fileName)
{
	uint32_t crc;
	if(this->FileCrc(fileName, &crc, 1048576))
	{
		return crc;
	}
	else return 0xffffffff; //While we return this as an error code, it is infact a valid CRC!
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Calculates the CRC32 of a file using the a default buffer size of 1MB.

	Note: The buffer size DOES NOT affect the resulting CRC,
			it has been provided for performance purposes only.
*/

uint32_t Crc32::FileCrc(const char *fileName, size_t bufferSize)
{
	uint32_t crc;
	if(this->FileCrc(fileName, &crc, bufferSize))
	{
		return crc;
	}
	else 
        return 0xffffffff; //While we return this as an error code, it is infact a valid CRC!
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Calculates the CRC32 of a file using the a default buffer size of 1MB.
*/

bool Crc32::FileCrc(const char *fileName, uint32_t *crc)
{
	return this->FileCrc(fileName, crc, 1048576);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////