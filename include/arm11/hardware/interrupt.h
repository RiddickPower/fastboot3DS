#pragma once

/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2017 derrek, profi200
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "types.h"
#include "mem_map.h"
#include "arm.h"


#define CPU_II_REGS_BASE     (MPCORE_PRIV_REG_BASE + 0x100)
#define REG_CPU_II_CNT       *((vu32*)(CPU_II_REGS_BASE + 0x00))
#define REG_CPU_II_MASK      *((vu32*)(CPU_II_REGS_BASE + 0x04))
#define REG_CPU_II_BIN_POI   *((vu32*)(CPU_II_REGS_BASE + 0x08))
#define REG_CPU_II_AKN       *((vu32*)(CPU_II_REGS_BASE + 0x0C))
#define REG_CPU_II_EOI       *((vu32*)(CPU_II_REGS_BASE + 0x10))
#define REG_CPU_II_RUN_PRIO  *((vu32*)(CPU_II_REGS_BASE + 0x14))
#define REG_CPU_II_HIGH_PEN  *((vu32*)(CPU_II_REGS_BASE + 0x18))

#define GID_REGS_BASE        (MPCORE_PRIV_REG_BASE + 0x1000)
#define REG_GID_CNT          *((vu32*)(GID_REGS_BASE + 0x000))
#define REG_GID_CONTR_TYPE   *((vu32*)(GID_REGS_BASE + 0x004))
#define REGs_GID_ENA_SET      ((vu32*)(GID_REGS_BASE + 0x100))
#define REGs_GID_ENA_CLR      ((vu32*)(GID_REGS_BASE + 0x180))
#define REGs_GID_PEN_SET      ((vu32*)(GID_REGS_BASE + 0x200))
#define REGs_GID_PEN_CLR      ((vu32*)(GID_REGS_BASE + 0x280))
#define REGs_GID_ACTIVE_BIT   ((vu32*)(GID_REGS_BASE + 0x300))
#define REGs_GID_IPRIO        ((vu32*)(GID_REGS_BASE + 0x400))
#define REGs_GID_ITARG        ((vu32*)(GID_REGS_BASE + 0x800))
#define REGs_GID_ICONF        ((vu32*)(GID_REGS_BASE + 0xC00))
#define REGs_GID_LINE_LEV     ((vu32*)(GID_REGS_BASE + 0xD00))
#define REG_GID_SW_INT       *((vu32*)(GID_REGS_BASE + 0xF00))
#define REG_GID_PERI_INFO0   *((vu32*)(GID_REGS_BASE + 0xFE0))
#define REG_GID_PERI_INFO1   *((vu32*)(GID_REGS_BASE + 0xFE4))
#define REG_GID_PERI_INFO2   *((vu32*)(GID_REGS_BASE + 0xFE8))
#define REG_GID_PERI_INFO3   *((vu32*)(GID_REGS_BASE + 0xFEC))
#define REG_GID_PRIME_CELL0  *((vu32*)(GID_REGS_BASE + 0xFF0))
#define REG_GID_PRIME_CELL1  *((vu32*)(GID_REGS_BASE + 0xFF4))
#define REG_GID_PRIME_CELL2  *((vu32*)(GID_REGS_BASE + 0xFF8))
#define REG_GID_PRIME_CELL3  *((vu32*)(GID_REGS_BASE + 0xFFC))


typedef enum
{
	IRQ_MPCORE_SW0    = 0,
	IRQ_MPCORE_SW1    = 1,
	IRQ_MPCORE_SW2    = 2,
	IRQ_MPCORE_SW3    = 3,
	IRQ_MPCORE_SW4    = 4,
	IRQ_MPCORE_SW5    = 5,
	IRQ_MPCORE_SW6    = 6,
	IRQ_MPCORE_SW7    = 7,
	IRQ_MPCORE_SW8    = 8,
	IRQ_MPCORE_SW9    = 9,
	IRQ_MPCORE_SW10   = 10,
	IRQ_MPCORE_SW11   = 11,
	IRQ_MPCORE_SW12   = 12,
	IRQ_MPCORE_SW13   = 13,
	IRQ_MPCORE_SW14   = 14,
	IRQ_MPCORE_SW15   = 15,
	IRQ_TIMER         = 29,  // MPCore timer
	IRQ_WATCHDOG      = 30,  // MPCore watchdog
	IRQ_PSC0          = 40,
	IRQ_PSC1          = 41,
	IRQ_PDC0          = 42,  // aka VBlank0
	IRQ_PDC1          = 43,  // aka VBlank1
	IRQ_PPF           = 44,
	IRQ_P3D           = 45,  // This is a guess
	IRQ_PXI_SYNC      = 80,
	IRQ_PXI_UNK       = 81,  // Unknown what this is for
	IRQ_PXI_NOT_FULL  = 82,
	IRQ_PXI_NOT_EMPTY = 83,
	IRQ_I2C1          = 84,
	IRQ_I2C2          = 85,
	IRQ_PDN           = 88,
	IRQ_I2C3          = 92,
	IRQ_SHELL_OPENED  = 96,
	IRQ_SHELL_CLOSED  = 98,
	IRQ_TOUCHSCREEN   = 99,  // Triggers on hitting the touchscreen.
	IRQ_HEADPH_JACK   = 100, // Headphone jack. Triggers on both plugging in and out? GPIO reg 9, bit 8.
	IRQ_MCU_HID       = 113, // HOME/POWER pressed/released, shell opened/closed and WiFi switch pressed. GPIO reg 19, bit 9.
	IRQ_GAMECARD      = 117, // Gamecard inserted
	IRQ_PERF_MONITOR0 = 120, // Core 0 performance monitor. Triggers on any counter overflow
	IRQ_PERF_MONITOR1 = 121, // Core 1 performance monitor. Triggers on any counter overflow
	IRQ_PERF_MONITOR2 = 122, // Unconfirmed. Core 2 performance monitor. Triggers on any counter overflow
	IRQ_PERF_MONITOR3 = 123  // Unconfirmed. Core 3 performance monitor. Triggers on any counter overflow
} Interrupt;


// IRQ handler type.
// intSource: bit 10-12 CPU source ID (0 except for interrupt ID 0-15),
// bit 0-9 interrupt ID
typedef void (*IrqHandler)(u32 intSource);



/**
 * @brief      Initializes the generic interrupt controller.
 */
void IRQ_init(void);

/**
 * @brief      Registers a interrupt handler and enables the specified interrupt.
 *
 * @param[in]  id             The interrupt ID. Must be <128.
 * @param[in]  prio           The priority. 0 = highest, 14 = lowest, 15 = disabled
 * @param[in]  cpuMask        The CPU mask. Each of the 4 bits stands for 1 core. 0 means current CPU.
 * @param[in]  edgeTriggered  Set to true to make the interrupt edge triggered. false is level triggered.
 * @param[in]  handler        The interrupt handler to call.
 */
void IRQ_registerHandler(Interrupt id, u8 prio, u8 cpuMask, bool edgeTriggered, IrqHandler handler);

/**
 * @brief      Unregisters the interrupt handler and disables the specified interrupt.
 *
 * @param[in]  id    The interrupt ID. Must be <128.
 */
void IRQ_unregisterHandler(Interrupt id);

/**
 * @brief      Reenables a previously disabled but registered interrupt.
 *
 * @param[in]  id    The interrupt ID. Must be <128.
 */
void IRQ_enable(Interrupt id);

/**
 * @brief      Disables a previously registered interrupt temporarily.
 *
 * @param[in]  id    The interrupt ID. Must be <128.
 */
void IRQ_disable(Interrupt id);

/**
 * @brief      Sets the priority of an interrupt.
 *
 * @param[in]  id    The interrupt ID. Must be <128.
 * @param[in]  prio  The priority. 0 = highest, 14 = lowest, 15 = disabled
 */
void IRQ_setPriority(Interrupt id, u8 prio);

/**
 * @brief      Triggers a software interrupt for the specified CPUs.
 *
 * @param[in]  id       The interrupt ID. Must be <16.
 * @param[in]  cpuMask  The CPU mask. Each of the 4 bits stands for 1 core.
 */
void IRQ_softwareInterrupt(Interrupt id, u8 cpuMask);


static inline u32 enterCriticalSection(void)
{
	const u32 tmp = __getCpsr();
	__cpsid(i);
	return tmp & PSR_I;
}

static inline void leaveCriticalSection(u32 oldState)
{
	__setCpsr_c((__getCpsr() & ~PSR_I) | oldState);
}
