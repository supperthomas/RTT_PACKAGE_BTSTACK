import os
import rtconfig
from building import *

cwd = GetCurrentDir()

# add file
src = Split('''
platform/posix/btstack_stdin_posix.c
platform/posix/btstack_uart_block_posix.c
platform/posix/btstack_tlv_posix.c
platform/posix/btstack_run_loop_posix.c

chipset/bcm/btstack_chipset_bcm.c
chipset/bcm/btstack_chipset_bcm_download_firmware.c

src/btstack_util.c
src/btstack_run_loop.c
src/btstack_tlv.c
src/hci_dump.c
src/hci_cmd.c
src/ad_parser.c
src/btstack_linked_list.c
src/btstack_crypto.c


src/hci.c
src/l2cap.c
src/l2cap_signaling.c
src/btstack_memory.c
src/btstack_memory_pool.c
src/hci_transport_h4.c

src/classic/btstack_link_key_db_tlv.c

src/ble/le_device_db_tlv.c
src/ble/att_server.c
src/ble/sm.c
src/ble/att_dispatch.c
src/ble/att_db.c

src/ble/gatt-service/battery_service_server.c

port/posix-h4-bcm/btstack_main.c
rtt_adapter/rtt_btstack_adapter.c

src/btstack_crypto.c
''')


path =  [cwd]
path += [cwd + '/src']

path += [cwd + '/platform/posix']
path += [cwd + '/chipset/bcm']
path += [cwd + '/port/posix-h4-bcm']
path += [cwd + '/rtt_adapter']
path += [cwd + '/src/ble/gatt-service']

#MESH
if GetDepend(['PKG_BTSTACK_MESH']):
     path += [cwd + '/src/mesh']
     src += Split("""
			src/mesh/adv_bearer.c
			src/mesh/mesh_access.c
			src/mesh/beacon.c
			src/mesh/gatt_bearer.c
			src/mesh/mesh.c
			src/mesh/mesh_crypto.c
			src/mesh/mesh_configuration_client.c
			src/mesh/mesh_configuration_server.c
			src/mesh/mesh_foundation.c
			src/mesh/mesh_generic_default_transition_time_client.c
			src/mesh/mesh_generic_default_transition_time_server.c
			src/mesh/mesh_generic_level_client.c
			src/mesh/mesh_generic_level_server.c
			src/mesh/mesh_generic_on_off_client.c
			src/mesh/mesh_generic_on_off_server.c
			src/mesh/mesh_health_server.c
			src/mesh/mesh_iv_index_seq_number.c
			src/mesh/mesh_keys.c
			src/mesh/mesh_lower_transport.c
			src/mesh/mesh_network.c
			src/mesh/mesh_peer.c
			src/mesh/mesh_proxy.c
			src/mesh/mesh_node.c
			src/mesh/mesh_upper_transport.c
			src/mesh/mesh_virtual_addresses.c
			src/mesh/pb_adv.c
			src/mesh/pb_gatt.c
			src/mesh/provisioning.c
			src/mesh/provisioning_device.c
			src/mesh/provisioning_provisioner.c
			src/ble/gatt-service/mesh_provisioning_service_server.c
			src/ble/gatt-service/mesh_proxy_service_server.c
        """)  





#example

if GetDepend('PKG_BTSTACK_SAMPLE_DISABLE') == False:
     path += [cwd + '/example/inc']


if GetDepend(['PKG_BTSTACK_SAMPLE_LE_COUNTER']):
     example_src = Split("""
        example/gatt_counter.c
        """)
        
if GetDepend(['PKG_BTSTACK_SAMPLE_LE_DATA_CHANNEL_SERVER']):
     example_src = Split("""
        example/le_data_channel_server.c
        """)
        
if GetDepend(['PKG_BTSTACK_SAMPLE_LE_DATA_CHANNEL_SERVER']):
     example_src = Split("""
        example/le_data_channel_server.c
        """)
        
if GetDepend(['PKG_BTSTACK_SAMPLE_LE_BLUFI_SERVER']):
     example_src = Split("""
        rtt_adapter/rtt_btstack_gatt_blufi.c
        """)
        
if GetDepend(['PKG_BTSTACK_SAMPLE_LE_STREAMER']):
     example_src = Split("""
        example/nordic_spp_le_streamer.c
        src/ble/gatt-service/nordic_spp_service_server.c
        """)  
 
if GetDepend(['PKG_BTSTACK_SAMPLE_NORDIC_LE_COUNTER']):
     example_src = Split("""
        example/nordic_spp_le_counter.c
        src/ble/gatt-service/nordic_spp_service_server.c
        """)  

if GetDepend(['PKG_BTSTACK_SAMPLE_ATT_DELAYED_RESPONSE']):
     example_src = Split("""
        example/att_delayed_response.c
        """)  


if GetDepend(['PKG_BTSTACK_SAMPLE_SM_PAIRING_PERIPHERAL']):
     example_src = Split("""
        example/sm_pairing_peripheral.c
        """)       

if GetDepend(['PKG_BTSTACK_SAMPLE_GATT_STREAMER_PERIPHERAL']):
     example_src = Split("""
        example/gatt_streamer_server.c
        """)       
        
if GetDepend(['PKG_BTSTACK_SAMPLE_ANCS_CLIENT_DEMO']):
     example_src = Split("""
        example/ancs_client_demo.c
        src/ble/ancs_client.c
        src/ble/gatt_client.c
        """)   
        
if GetDepend(['PKG_BTSTACK_SAMPLE_MESH_DEMO']):
     example_src = Split("""
        example/mesh_node_demo.c
        """)  
        
if GetDepend(['PKG_BTSTACK_SAMPLE_TMALL_MESH_DEMO']):
     example_src = Split("""
        rtt_adapter/mesh_node_demo_tmall.c
        """)        
          
LOCAL_CCFLAGS = ''
if rtconfig.CROSS_TOOL == 'keil':
    LOCAL_CCFLAGS += ' --gnu'


group = DefineGroup('RTT_BTSTACK', src, depend = [''], CPPPATH = path, LOCAL_CCFLAGS = LOCAL_CCFLAGS)

if GetDepend('PKG_BTSTACK_SAMPLE_DISABLE') == False:
    group = group + DefineGroup('btstack_example', example_src, depend = [''], CPPPATH = path)
    

Return('group')
