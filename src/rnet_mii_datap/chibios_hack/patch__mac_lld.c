--- /tmp/DdVa9P_mac_lld.c
+++ /work/Projects/stm32/ChibiOS/os/hal/platforms/STM32/mac_lld.c
@@ -102,7 +102,22 @@
  * @return              The PHY register content.
  */
 static uint32_t mii_read(MACDriver *macp, uint32_t reg) {
-
+    
+    
+    switch (reg)  {
+        case 0: 
+            return  0b0011000100000001;                //bmcr
+            break;
+        case 1:
+            return  0b0111100001101101;               // bmsr
+            break;
+        case 5:
+            return  0b0000000111100001;               // bmsr auto-neg link parterability
+            break;
+        default:
+            return  0b0;
+            break;
+    }
   ETH->MACMIIAR = macp->phyaddr | (reg << 6) | MACMIIDR_CR | ETH_MACMIIAR_MB;
   while ((ETH->MACMIIAR & ETH_MACMIIAR_MB) != 0)
     ;
@@ -397,9 +412,12 @@
 msg_t mac_lld_get_transmit_descriptor(MACDriver *macp,
                                       MACTransmitDescriptor *tdp) {
   stm32_eth_tx_descriptor_t *tdes;
-
-  if (!macp->link_up)
+  BaseSequentialStream *chp   =  (BaseSequentialStream *)&SD1;
+
+  if (!macp->link_up) {
+    chprintf(chp, "no link up\r\n");
     return RDY_TIMEOUT;
+  }
 
   chSysLock();
 
@@ -410,6 +428,7 @@
      another thread.*/
   if (tdes->tdes0 & (STM32_TDES0_OWN | STM32_TDES0_LOCKED)) {
     chSysUnlock();
+    chprintf(chp, "descriptor issues\r\n");
     return RDY_TIMEOUT;
   }
 
@@ -552,12 +571,15 @@
 bool_t mac_lld_poll_link_status(MACDriver *macp) {
   uint32_t maccr, bmsr, bmcr;
 
+  BaseSequentialStream *chp   =  (BaseSequentialStream *)&SD1;
   maccr = ETH->MACCR;
 
   /* PHY CR and SR registers read.*/
   (void)mii_read(macp, MII_BMSR);
-  bmsr = mii_read(macp, MII_BMSR);
-  bmcr = mii_read(macp, MII_BMCR);
+  /*bmsr = mii_read(macp, MII_BMSR);*/
+  bmsr = 0b0111100001101101;
+  /*bmcr = mii_read(macp, MII_BMCR);*/
+  bmcr = 0b0011000100000001;                
 
   /* Check on auto-negotiation mode.*/
   if (bmcr & BMCR_ANENABLE) {
@@ -565,8 +587,10 @@
 
     /* Auto-negotiation must be finished without faults and link established.*/
     if ((bmsr & (BMSR_LSTATUS | BMSR_RFAULT | BMSR_ANEGCOMPLETE)) !=
-        (BMSR_LSTATUS | BMSR_ANEGCOMPLETE))
+        (BMSR_LSTATUS | BMSR_ANEGCOMPLETE)) {
+        chprintf(chp, "\t***auto nego fail\r\n");
       return macp->link_up = FALSE;
+    }
 
     /* Auto-negotiation enabled, checks the LPA register.*/
     lpa = mii_read(macp, MII_LPA);
@@ -585,8 +609,10 @@
   }
   else {
     /* Link must be established.*/
-    if (!(bmsr & BMSR_LSTATUS))
-      return macp->link_up = FALSE;
+      if (!(bmsr & BMSR_LSTATUS)) {
+        chprintf(chp, "\t***link est fail \r\n");
+          return macp->link_up = FALSE;
+      }
 
     /* Check on link speed.*/
     if (bmcr & BMCR_SPEED100)
@@ -605,6 +631,7 @@
   ETH->MACCR = maccr;
 
   /* Returns the link status.*/
+  chprintf(chp, "exit with link up\r\n");
   return macp->link_up = TRUE;
 }
 
