PRIMARY_SERVICE(service_gap, gBleSig_GenericAccessProfile_d)
    CHARACTERISTIC(char_device_name, gBleSig_GapDeviceName_d, (gGattCharPropRead_c | gGattCharPropWrite_c) )
        VALUE(value_device_name, gBleSig_GapDeviceName_d, (gPermissionFlagReadable_c | gPermissionFlagWritable_c), 6+12, "BICOSE            ")
    CHARACTERISTIC(char_appearance, gBleSig_GapAppearance_d, (gGattCharPropRead_c) )
            VALUE(value_appearance, gBleSig_GapAppearance_d, (gPermissionFlagReadable_c), 2, UuidArray(gUnknown_c))
    CHARACTERISTIC(char_ppcp, gBleSig_GapPpcp_d, (gGattCharPropRead_c) )
        VALUE(value_ppcp, gBleSig_GapPpcp_d, (gPermissionFlagReadable_c), 8, 0x50, 0x00, 0xA0, 0x00, 0x00, 0x00, 0xE8, 0x03)

PRIMARY_SERVICE(service_gatt, gBleSig_GenericAttributeProfile_d)
    CHARACTERISTIC(char_service_changed, gBleSig_GattServiceChanged_d, (gGattCharPropIndicate_c) )
        VALUE(value_service_changed, gBleSig_GattServiceChanged_d, (gPermissionNone_c), 4, 0x00, 0x00, 0x00, 0x00)
        CCCD(cccd_service_changed)

PRIMARY_SERVICE(service_ccc_dk, 0xFFF5)
	CHARACTERISTIC(char_ccc_dk,    0xFFE1, (gGattCharPropRead_c))
		VALUE(value_ccc_dk, 0xFFE1, (gPermissionFlagReadable_c), 16, 0xA3, 0x80, 0xF9, 0xE5, 0x1E, 0x6B, 0xE4, 0x8B, 0x3A, 0x4B, 0x23, 0x9E, 0x30, 0xA1, 0xB5, 0xD3)
	CHARACTERISTIC_UUID128(char_ccc_dk_read, uuid_char_ccc_dk, (gGattCharPropRead_c) )
		VALUE_UUID128(value_ccc_dk_read, uuid_char_ccc_dk, (gPermissionFlagReadable_c), 2, UuidArray(gCcc_SPSM_c))


PRIMARY_SERVICE(service_yqdk, 0xFFE0)
	CHARACTERISTIC(char_yqdk_1,    0xFFE1, (gGattCharPropWrite_c))
		VALUE_VARLEN(value_yqdk_1, 0xFFE1, (gPermissionFlagWritable_c), 205, 1, 0x00)
	CHARACTERISTIC(char_yqdk_2,    0xFFE2, (gGattCharPropRead_c | gGattCharPropNotify_c))
		VALUE_VARLEN(value_yqdk_2, 0xFFE2, (gPermissionFlagReadable_c), 205, 1, 0x00)
		CCCD(cccd_yqdk_2)

PRIMARY_SERVICE(service_persional, 0xFFE3)
	CHARACTERISTIC(char_persional_1,    0xFFE4, (gGattCharPropWrite_c))
		VALUE_VARLEN(value_persional_1, 0xFFE4, (gPermissionFlagWritable_c), 205, 1, 0x00)
	CHARACTERISTIC(char_persional_2,    0xFFE5, (gGattCharPropRead_c | gGattCharPropNotify_c))
		VALUE_VARLEN(value_persional_2, 0xFFE5, (gPermissionFlagReadable_c), 205, 1, 0x00)
		CCCD(cccd_persional_2)
		

		
