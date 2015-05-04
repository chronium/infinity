#ifndef INFINITY_ARCH_PCI_H
#define INFINITY_ARCH_PCI_H

#include <stdint.h>

uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pci_check_vendor(uint8_t bus, uint8_t slot);

#endif
