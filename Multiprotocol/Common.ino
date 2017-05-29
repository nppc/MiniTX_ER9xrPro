/*
 This project is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Multiprotocol is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Multiprotocol.  If not, see <http://www.gnu.org/licenses/>.
 */
/************************/
/**  Convert routines  **/
/************************/
// Channel value is converted to 8bit values full scale
uint8_t convert_channel_8b(uint8_t num)
{
	return (uint8_t) (map(limit_channel_100(num),servo_min_100,servo_max_100,0,255));	
}

// Channel value is converted to 8bit values to provided values scale
uint8_t convert_channel_8b_scale(uint8_t num,uint8_t min,uint8_t max)
{
	return (uint8_t) (map(limit_channel_100(num),servo_min_100,servo_max_100,min,max));	
}

// Channel value is converted sign + magnitude 8bit values
uint8_t convert_channel_s8b(uint8_t num)
{
	uint8_t ch;
	ch = convert_channel_8b(num);
	return (ch < 128 ? 127-ch : ch);	
}

// Channel value is converted to 10bit values
uint16_t convert_channel_10b(uint8_t num)
{
	return (uint16_t) (map(limit_channel_100(num),servo_min_100,servo_max_100,1,1023));
}

// Channel value is multiplied by 1.5
uint16_t convert_channel_frsky(uint8_t num)
{
	return Servo_data[num] + Servo_data[num]/2;
}

// Channel value is converted for HK310
void convert_channel_HK310(uint8_t num, uint8_t *low, uint8_t *high)
{
	uint16_t temp=0xFFFF-(4*Servo_data[num])/3;
	*low=(uint8_t)(temp&0xFF);
	*high=(uint8_t)(temp>>8);
}

// Channel value is converted to 16bit values
uint16_t convert_channel_16b(uint8_t num, int16_t out_min, int16_t out_max)
{
	return (uint16_t) (map(limit_channel_100(num),servo_min_100,servo_max_100,out_min,out_max));
}

// Channel value is converted to 16bit values with no limit
uint16_t convert_channel_16b_nolim(uint8_t num, int16_t out_min, int16_t out_max)
{
	return (uint16_t) (map(Servo_data[num],servo_min_100,servo_max_100,out_min,out_max));
}

// Channel value is limited to PPM_100
uint16_t limit_channel_100(uint8_t ch)
{
	if(Servo_data[ch]>servo_max_100)
		return servo_max_100;
	else
		if (Servo_data[ch]<servo_min_100)
			return servo_min_100;
	return Servo_data[ch];
}

/******************************/
/**  FrSky D and X routines  **/
/******************************/
#if defined(FRSKYD_CC2500_INO) || defined(FRSKYX_CC2500_INO)
enum {
	FRSKY_BIND		= 0,
	FRSKY_BIND_DONE	= 1000,
	FRSKY_DATA1,
	FRSKY_DATA2,
	FRSKY_DATA3,
	FRSKY_DATA4,
	FRSKY_DATA5
};

void Frsky_init_hop(void)
{
	uint8_t val;
	uint8_t channel = rx_tx_addr[0]&0x07;
	uint8_t channel_spacing = rx_tx_addr[1];
	//Filter bad tables
	if(channel_spacing<0x02) channel_spacing+=0x02;
	if(channel_spacing>0xE9) channel_spacing-=0xE7;
	if(channel_spacing%0x2F==0) channel_spacing++;
		
	hopping_frequency[0]=channel;
	for(uint8_t i=1;i<50;i++)
	{
		channel=(channel+channel_spacing) % 0xEB;
		val=channel;
		if((val==0x00) || (val==0x5A) || (val==0xDC))
			val++;
		hopping_frequency[i]=i>46?0:val;
	}
}
#endif
/******************************/
/**  FrSky V, D and X routines  **/
/******************************/
#if defined(FRSKYV_CC2500_INO) || defined(FRSKYD_CC2500_INO) || defined(FRSKYX_CC2500_INO)
	const PROGMEM uint8_t FRSKY_common_startreg_cc2500_conf[]= {
		 CC2500_02_IOCFG0 ,		
		 CC2500_00_IOCFG2 ,
		 CC2500_17_MCSM1 ,
		 CC2500_18_MCSM0 ,
		 CC2500_06_PKTLEN ,
		 CC2500_07_PKTCTRL1 ,
		 CC2500_08_PKTCTRL0 ,
		 CC2500_3E_PATABLE ,
		 CC2500_0B_FSCTRL1 ,
		 CC2500_0C_FSCTRL0 ,	// replaced by option value
		 CC2500_0D_FREQ2 ,	
		 CC2500_0E_FREQ1 ,
		 CC2500_0F_FREQ0 ,
		 CC2500_10_MDMCFG4 ,		
		 CC2500_11_MDMCFG3 ,
		 CC2500_12_MDMCFG2 ,
		 CC2500_13_MDMCFG1 ,
		 CC2500_14_MDMCFG0 ,
		 CC2500_15_DEVIATN  };

	#if defined(FRSKYV_CC2500_INO)
		const PROGMEM uint8_t FRSKYV_cc2500_conf[]= {
		/*02_IOCFG0*/  	 0x06 ,		
		/*00_IOCFG2*/  	 0x06 ,
		/*17_MCSM1*/   	 0x0c ,
		/*18_MCSM0*/   	 0x18 ,
		/*06_PKTLEN*/  	 0xff ,
		/*07_PKTCTRL1*/	 0x04 ,
		/*08_PKTCTRL0*/	 0x05 ,
		/*3E_PATABLE*/ 	 0xfe ,
		/*0B_FSCTRL1*/ 	 0x08 ,
		/*0C_FSCTRL0*/ 	 0x00 ,
		/*0D_FREQ2*/   	 0x5c ,	
		/*0E_FREQ1*/   	 0x58 ,
		/*0F_FREQ0*/   	 0x9d ,
		/*10_MDMCFG4*/ 	 0xAA ,		
		/*11_MDMCFG3*/ 	 0x10 ,
		/*12_MDMCFG2*/ 	 0x93 ,
		/*13_MDMCFG1*/ 	 0x23 ,
		/*14_MDMCFG0*/ 	 0x7a ,
		/*15_DEVIATN*/ 	 0x41  };
	#endif

	#if defined(FRSKYD_CC2500_INO)
		const PROGMEM uint8_t FRSKYD_cc2500_conf[]= {
		/*02_IOCFG0*/  	 0x06 ,		
		/*00_IOCFG2*/  	 0x06 ,
		/*17_MCSM1*/   	 0x0c ,
		/*18_MCSM0*/   	 0x18 ,
		/*06_PKTLEN*/  	 0x19 ,
		/*07_PKTCTRL1*/	 0x04 ,
		/*08_PKTCTRL0*/	 0x05 ,
		/*3E_PATABLE*/ 	 0xff ,
		/*0B_FSCTRL1*/ 	 0x08 ,
		/*0C_FSCTRL0*/ 	 0x00 ,
		/*0D_FREQ2*/   	 0x5c ,	
		/*0E_FREQ1*/   	 0x76 ,
		/*0F_FREQ0*/   	 0x27 ,
		/*10_MDMCFG4*/ 	 0xAA ,		
		/*11_MDMCFG3*/ 	 0x39 ,
		/*12_MDMCFG2*/ 	 0x11 ,
		/*13_MDMCFG1*/ 	 0x23 ,
		/*14_MDMCFG0*/ 	 0x7a ,
		/*15_DEVIATN*/ 	 0x42  };
	#endif

	#if defined(FRSKYX_CC2500_INO)
		const PROGMEM uint8_t FRSKYX_cc2500_conf[]= {
	//FRSKYX
		/*02_IOCFG0*/  	 0x06 ,		
		/*00_IOCFG2*/  	 0x06 ,
		/*17_MCSM1*/   	 0x0c ,
		/*18_MCSM0*/   	 0x18 ,
		/*06_PKTLEN*/  	 0x1E ,
		/*07_PKTCTRL1*/	 0x04 ,
		/*08_PKTCTRL0*/	 0x01 ,
		/*3E_PATABLE*/ 	 0xff ,
		/*0B_FSCTRL1*/ 	 0x0A ,
		/*0C_FSCTRL0*/ 	 0x00 ,
		/*0D_FREQ2*/   	 0x5c ,	
		/*0E_FREQ1*/   	 0x76 ,
		/*0F_FREQ0*/   	 0x27 ,
		/*10_MDMCFG4*/ 	 0x7B ,		
		/*11_MDMCFG3*/ 	 0x61 ,
		/*12_MDMCFG2*/ 	 0x13 ,
		/*13_MDMCFG1*/ 	 0x23 ,
		/*14_MDMCFG0*/ 	 0x7a ,
		/*15_DEVIATN*/ 	 0x51  };
		const PROGMEM uint8_t FRSKYXEU_cc2500_conf[]= {
		/*02_IOCFG0*/  	 0x06 ,		
		/*00_IOCFG2*/  	 0x06 ,
		/*17_MCSM1*/   	 0x0E ,
		/*18_MCSM0*/   	 0x18 ,
		/*06_PKTLEN*/  	 0x23 ,
		/*07_PKTCTRL1*/	 0x04 ,
		/*08_PKTCTRL0*/	 0x01 ,
		/*3E_PATABLE*/ 	 0xff ,
		/*0B_FSCTRL1*/ 	 0x08 ,
		/*0C_FSCTRL0*/ 	 0x00 ,
		/*0D_FREQ2*/   	 0x5c ,	
		/*0E_FREQ1*/   	 0x80 ,
		/*0F_FREQ0*/   	 0x00 ,
		/*10_MDMCFG4*/ 	 0x7B ,		
		/*11_MDMCFG3*/ 	 0xF8 ,
		/*12_MDMCFG2*/ 	 0x03 ,
		/*13_MDMCFG1*/ 	 0x23 ,
		/*14_MDMCFG0*/ 	 0x7a ,
		/*15_DEVIATN*/ 	 0x53  };
	#endif

	const PROGMEM uint8_t FRSKY_common_end_cc2500_conf[][2]= {
		{ CC2500_19_FOCCFG,   0x16 },
		{ CC2500_1A_BSCFG,    0x6c },	
		{ CC2500_1B_AGCCTRL2, 0x43 },
		{ CC2500_1C_AGCCTRL1, 0x40 },
		{ CC2500_1D_AGCCTRL0, 0x91 },
		{ CC2500_21_FREND1,   0x56 },
		{ CC2500_22_FREND0,   0x10 },
		{ CC2500_23_FSCAL3,   0xa9 },
		{ CC2500_24_FSCAL2,   0x0A },
		{ CC2500_25_FSCAL1,   0x00 },
		{ CC2500_26_FSCAL0,   0x11 },
		{ CC2500_29_FSTEST,   0x59 },
		{ CC2500_2C_TEST2,    0x88 },
		{ CC2500_2D_TEST1,    0x31 },
		{ CC2500_2E_TEST0,    0x0B },
		{ CC2500_03_FIFOTHR,  0x07 },
		{ CC2500_09_ADDR,     0x00 } };

	void FRSKY_init_cc2500(const uint8_t *ptr)
	{
		for(uint8_t i=0;i<19;i++)
		{
			uint8_t reg=pgm_read_byte_near(&FRSKY_common_startreg_cc2500_conf[i]);
			uint8_t val=pgm_read_byte_near(&ptr[i]);
			if(reg==CC2500_0C_FSCTRL0)
				val=option;
			CC2500_WriteReg(reg,val);
		}
		prev_option = option ;		// Save option to monitor FSCTRL0 change
		for(uint8_t i=0;i<17;i++)
		{
			uint8_t reg=pgm_read_byte_near(&FRSKY_common_end_cc2500_conf[i][0]);
			uint8_t val=pgm_read_byte_near(&FRSKY_common_end_cc2500_conf[i][1]);
			CC2500_WriteReg(reg,val);
		}
		CC2500_SetTxRxMode(TX_EN);
		CC2500_SetPower();
		CC2500_Strobe(CC2500_SIDLE);    // Go to idle...
	}
#endif
