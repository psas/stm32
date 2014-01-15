--- /tmp/VqbTby_mac_lld.c
+++ /work/Projects/stm32/ChibiOS/os/hal/platforms/STM32/mac_lld.c
@@ -102,11 +102,28 @@
  * @return              The PHY register content.
  */
 static uint32_t mii_read(MACDriver *macp, uint32_t reg) {
-
+    
+#ifdef BOARD_PSAS_ROCKETNET_HUB_1_0    
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
+#else
   ETH->MACMIIAR = macp->phyaddr | (reg << 6) | MACMIIDR_CR | ETH_MACMIIAR_MB;
   while ((ETH->MACMIIAR & ETH_MACMIIAR_MB) != 0)
     ;
   return ETH->MACMIIDR;
+#endif
 }
 
 #if !defined(BOARD_PHY_ADDRESS)
