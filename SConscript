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
src/ble/gatt-service/device_information_service_server.c

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

#CLASS
if GetDepend(['ENABLE_CLASSIC']):
     path += [cwd + '/src/class']
     src += Split("""
        src/classic/sdp_util.c
        src/classic/gatt_sdp.c
        src/classic/spp_server.c
        src/classic/rfcomm.c
        src/classic/bnep.c
        src/classic/sdp_server.c
        src/classic/device_id_server.c
        """)

if GetDepend(['PKG_BTSTACK_SDP_CLIENT']):
    src += Split("""
		src/classic/sdp_client.c
		src/classic/sdp_client_rfcomm.c
        """)


#encode decode
if GetDepend(['PKG_BTSTACK_AUDIO']):
    src += Split("""
        src/btstack_audio.c
        """)



if GetDepend(['PKG_BTSTACK_SBC_ENC']):
     path += [cwd + '/3rd-party/bluedroid/encoder/include']
     src += Split("""
        3rd-party/bluedroid/encoder/srce/sbc_analysis.c          
		3rd-party/bluedroid/encoder/srce/sbc_dct.c               
		3rd-party/bluedroid/encoder/srce/sbc_dct_coeffs.c        
		3rd-party/bluedroid/encoder/srce/sbc_enc_bit_alloc_mono.c
		3rd-party/bluedroid/encoder/srce/sbc_enc_bit_alloc_ste.c 
		3rd-party/bluedroid/encoder/srce/sbc_enc_coeffs.c        
		3rd-party/bluedroid/encoder/srce/sbc_encoder.c           
		3rd-party/bluedroid/encoder/srce/sbc_packing.c  
        src/classic/btstack_sbc_encoder_bluedroid.c
        src/classic/hfp_msbc.c
        """)
        
if GetDepend(['PKG_BTSTACK_SBC_DEC']):
     path += [cwd + '/3rd-party/bluedroid/decoder/include']
     src += Split("""
        3rd-party/bluedroid/decoder/srce/alloc.c
        3rd-party/bluedroid/decoder/srce/bitalloc.c
        3rd-party/bluedroid/decoder/srce/bitalloc-sbc.c
        3rd-party/bluedroid/decoder/srce/bitstream-decode.c
        3rd-party/bluedroid/decoder/srce/decoder-oina.c
        3rd-party/bluedroid/decoder/srce/decoder-private.c
        3rd-party/bluedroid/decoder/srce/decoder-sbc.c
        3rd-party/bluedroid/decoder/srce/dequant.c
        3rd-party/bluedroid/decoder/srce/framing.c
        3rd-party/bluedroid/decoder/srce/framing-sbc.c
        3rd-party/bluedroid/decoder/srce/oi_codec_version.c
        3rd-party/bluedroid/decoder/srce/synthesis-sbc.c
        3rd-party/bluedroid/decoder/srce/synthesis-dct8.c
        3rd-party/bluedroid/decoder/srce/synthesis-8-generated.c
        src/classic/btstack_sbc_plc.c
        src/classic/btstack_sbc_decoder_bluedroid.c
        """)        
#CVSD
if GetDepend(['PKG_BTSTACK_CVSD']):
     src += Split("""
        src/classic/btstack_cvsd_plc.c
        """)
        
        
        
#MESH
if GetDepend(['ENABLE_MESH']):
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

#===================ble service==============
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
        
if GetDepend(['PKG_BTSTACK_SAMPLE_UBLOX_LE_COUNTER']):
     example_src = Split("""
        example/ublox_spp_le_counter.c
        src/ble/gatt-service/device_information_service_server
        src/ble/gatt-service/ublox_spp_service_server.c
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
#==============hogp ble service=============

if GetDepend(['PKG_BTSTACK_SAMPLE_HOGP_KEYBOARD_DEMO']):
     example_src = Split("""
        example/hog_keyboard_demo.c
        src/ble/gatt-service/hids_device.c
        src/btstack_ring_buffer.c
        """)   
        
if GetDepend(['PKG_BTSTACK_SAMPLE_HOGP_MOUSER_DEMO']):
     example_src = Split("""
        example/hog_mouse_demo.c
        src/ble/gatt-service/hids_device.c
        """)        
#===========mesh=========================       
if GetDepend(['PKG_BTSTACK_SAMPLE_MESH_DEMO']):
     example_src = Split("""
        example/mesh_node_demo.c
        """)  
        
if GetDepend(['PKG_BTSTACK_SAMPLE_TMALL_MESH_DEMO']):
     example_src = Split("""
        rtt_adapter/mesh_node_demo_tmall.c
        """)        
          

#======dual mode==============
if GetDepend(['PKG_BTSTACK_SAMPLE_SPP_GATT_COUNTER_DEMO']):
     example_src = Split("""
        src/ble/gatt-service/device_information_service_server.c
        src/ble/gatt-service/nordic_spp_service_server.c
        example/spp_and_gatt_counter.c
        """)
        
if GetDepend(['PKG_BTSTACK_SAMPLE_SPP_GATT_STREAMER_DEMO']):
     example_src = Split("""
        src/ble/gatt-service/nordic_spp_service_server.c
        example/nordic_spp_le_streamer.c
        """)
#================CLASSIC SPP==========
if GetDepend(['PKG_BTSTACK_SAMPLE_SPP_COUNTER_DEMO']):
     example_src = Split("""
        example/spp_counter.c
        """)
if GetDepend(['PKG_BTSTACK_SAMPLE_SPP_STREAMER_DEMO']):
     example_src = Split("""
        example/spp_streamer.c
        """)
        
#================CLASSIC HFP==========
if GetDepend(['PKG_BTSTACK_SAMPLE_HFP_HF_DEMO']):
     example_src = Split("""
        platform/posix/wav_util.c
        example/sco_demo_util.c
        src/btstack_ring_buffer.c
        src/classic/hfp.c
        src/classic/hfp_hf.c
        example/hfp_hf_demo.c
        """)
        
if GetDepend(['PKG_BTSTACK_SAMPLE_HFP_AG_DEMO']):
     example_src = Split("""
        platform/posix/wav_util.c
        example/sco_demo_util.c
        src/btstack_ring_buffer.c
        src/classic/hfp.c
        src/classic/hfp_gsm_model.c
        src/classic/hfp_ag.c
        example/hfp_ag_demo.c
        """)
        
if GetDepend(['PKG_BTSTACK_SAMPLE_HSP_HS_DEMO']):
     example_src = Split("""
        platform/posix/wav_util.c
        example/sco_demo_util.c
        src/btstack_ring_buffer.c
        src/classic/hsp_hs.c
        example/hsp_hs_demo.c
        """)
        
if GetDepend(['PKG_BTSTACK_SAMPLE_HSP_AG_DEMO']):
     example_src = Split("""
        platform/posix/wav_util.c
        example/sco_demo_util.c
        src/btstack_ring_buffer.c
        src/classic/hsp_ag.c
        example/hsp_ag_demo.c
        """)
        
        
LOCAL_CCFLAGS = ''
if rtconfig.PLATFORM in ['gcc', 'armclang']:
    LOCAL_CCFLAGS += ' -std=gnu99'
elif rtconfig.PLATFORM in ['armcc']:
    LOCAL_CCFLAGS += ' --c99 --gnu'


group = DefineGroup('RTT_BTSTACK', src, depend = [''], CPPPATH = path, LOCAL_CCFLAGS = LOCAL_CCFLAGS)

if GetDepend('PKG_BTSTACK_SAMPLE_DISABLE') == False:
    group = group + DefineGroup('btstack_example', example_src, depend = [''], CPPPATH = path)
    

Return('group')
