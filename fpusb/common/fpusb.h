#ifndef __FPUSB_H
#define __FPUSB_H

#define VID 0x16c0
#define PID 0x05dc

/* Ordres fil pilote :
 * Confort  : pas de signal             REQ_CONF 1
 * Reduit   : pleine alternance         REQ_REDU 2
 * Hors gel : 1/2 alternance positive   REQ_HGEL 3
 * Arret    : 1/2 alternance negative   REQ_ARRT 4
 */
#define REQ_CONF 1
#define REQ_HGEL 2
#define REQ_ARRT 3
#define REQ_REDU 4

/* radiateurs */

#endif // __FPUSB_H