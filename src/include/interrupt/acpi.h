#ifndef INTERRUPT_ACPI_H
#define INTERRUPT_ACPI_H

#include <stdint.h>

typedef struct _RSDP_t RSDP_t;
typedef struct _ACPISDTHeader ACPISDTHeader;
typedef struct _RSDT RSDT;
typedef struct _GenericAddressStructure GenericAddressStructure;
typedef struct _FADT FADT;

// RSDP is a structure which contains a pointer to the RSDT and some metadata
// struct taken from https://wiki.osdev.org/RSDP
struct _RSDP_t {
  char Signature[8];
  uint8_t Checksum;
  char OEMID[6];
  uint8_t Revision;
  uint32_t RsdtAddress;
} __attribute__((packed));

// The standard header is at the beginning of every ACPI system description
// table struct taken from https://wiki.osdev.org/RSDT
struct _ACPISDTHeader {
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
};

// The RSDT is unique among ACPI system description tables, in that its purpose
// is to point To all the other system descriptions struct taken from
// https://wiki.osdev.org/RSDT
struct _RSDT {
  ACPISDTHeader h;
  // There should be an array of ((h.Length - sizeof(h)) / 4)
  // Pointers to ACPI system description tables here
  uint32_t *PointerToOtherSDT;
};

// Generic address structure used by the FADT
// https://wiki.osdev.org/FADT
struct _GenericAddressStructure {
  uint8_t AddressSpace;
  uint8_t BitWidth;
  uint8_t BitOffset;
  uint8_t AccessSize;
  uint64_t Address;
};

// Fixed ACPI Description Table structure
// struct taken from https://wiki.osdev.org/FADT
struct _FADT {
  ACPISDTHeader h;
  uint32_t FirmwareCtrl;
  uint32_t Dsdt;

  // field used in ACPI 1.0; no longer in use, for compatibility only
  uint8_t Reserved;

  uint8_t PreferredPowerManagementProfile;
  uint16_t SCI_Interrupt;
  uint32_t SMI_CommandPort;
  uint8_t AcpiEnable;
  uint8_t AcpiDisable;
  uint8_t S4BIOS_REQ;
  uint8_t PSTATE_Control;
  uint32_t PM1aEventBlock;
  uint32_t PM1bEventBlock;
  uint32_t PM1aControlBlock;
  uint32_t PM1bControlBlock;
  uint32_t PM2ControlBlock;
  uint32_t PMTimerBlock;
  uint32_t GPE0Block;
  uint32_t GPE1Block;
  uint8_t PM1EventLength;
  uint8_t PM1ControlLength;
  uint8_t PM2ControlLength;
  uint8_t PMTimerLength;
  uint8_t GPE0Length;
  uint8_t GPE1Length;
  uint8_t GPE1Base;
  uint8_t CStateControl;
  uint16_t WorstC2Latency;
  uint16_t WorstC3Latency;
  uint16_t FlushSize;
  uint16_t FlushStride;
  uint8_t DutyOffset;
  uint8_t DutyWidth;
  uint8_t DayAlarm;
  uint8_t MonthAlarm;
  uint8_t Century;

  // reserved in ACPI 1.0; used since ACPI 2.0+
  uint16_t BootArchitectureFlags;

  uint8_t Reserved2;
  uint32_t Flags;

  // 12 byte structure; see below for details
  GenericAddressStructure ResetReg;

  uint8_t ResetValue;
  uint8_t Reserved3[3];

  // 64bit pointers - Available on ACPI 2.0+
  uint64_t X_FirmwareControl;
  uint64_t X_Dsdt;

  GenericAddressStructure X_PM1aEventBlock;
  GenericAddressStructure X_PM1bEventBlock;
  GenericAddressStructure X_PM1aControlBlock;
  GenericAddressStructure X_PM1bControlBlock;
  GenericAddressStructure X_PM2ControlBlock;
  GenericAddressStructure X_PMTimerBlock;
  GenericAddressStructure X_GPE0Block;
  GenericAddressStructure X_GPE1Block;
};

#endif // INTERRUPT_ACPI_H
