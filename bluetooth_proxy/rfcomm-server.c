#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

#include "proto.h"

#define RFCOMM_CHANNEL	3

void sdp_add_lang_attr(sdp_record_t *rec)
{
	sdp_lang_attr_t base_lang;
	sdp_list_t *langs;

	base_lang.code_ISO639 = (0x65 << 8) | 0x6e;
	base_lang.encoding = 106;
	base_lang.base_offset = SDP_PRIMARY_LANG_BASE;

	langs = sdp_list_append(0, &base_lang);
	sdp_set_lang_attr(rec, langs);
	sdp_list_free(langs, NULL);
}

sdp_session_t *register_service1()
{
    uint16_t lp = 0x1;
    const char *service_name = "Wireless iAP";
    const char *service_dsc = NULL;
    const char *service_prov = NULL;
    sdp_profile_desc_t profile;

    uuid_t root_uuid, l2cap_uuid, sdp_uuid, svc_uuid, class_uuid;
    sdp_list_t *l2cap_list = 0, 
               *sdp_list = 0,
               *root_list = 0,
               *proto_list = 0, 
               *access_proto_list = 0,
	       *pfseq = 0;
    sdp_data_t *channel = 0, *psm = 0;

    sdp_record_t *record = sdp_record_alloc();

    /* Force the record to be 0x1000f */
    record->handle = 0x10001;

    // set the general service ID
    sdp_uuid16_create( &svc_uuid, 0x1200 );
//    sdp_set_service_id( record, svc_uuid );

    // set the Service class ID
    sdp_list_t service_class = {NULL, &svc_uuid};
    sdp_set_service_classes( record, &service_class);

    // make the service record publicly browsable
//    sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
//    root_list = sdp_list_append(0, &root_uuid);
//    sdp_set_browse_groups( record, root_list );

//    sdp_uuid16_create(&class_uuid, PNP_INFO_SVCLASS_ID);
//    class_list = sdp_list_append(0, &class_uuid);
//    sdp_set_service_classes(record, class_list);

    // set l2cap information
    sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
    l2cap_list = sdp_list_append( 0, &l2cap_uuid );
    psm = sdp_data_alloc(SDP_UINT16, &lp);
    l2cap_list = sdp_list_append(l2cap_list, psm);
    proto_list = sdp_list_append( 0, l2cap_list );

    // set sdp information
    sdp_uuid16_create(&sdp_uuid, SDP_UUID);
    sdp_list = sdp_list_append( 0, &sdp_uuid );
    proto_list = sdp_list_append( proto_list, sdp_list );

    // attach protocol information to service record
    access_proto_list = sdp_list_append( 0, proto_list );
    sdp_set_access_protos( record, access_proto_list );

    // set the name, provider, and description
//    sdp_set_info_attr(record, service_name, service_prov, service_dsc);
    sdp_add_lang_attr(record);
//    sdp_set_service_avail(record, 0xff);
    
    uint16_t a200 = 0x0103;
    uint16_t a201 = 0x0039;
    uint16_t a202 = 0x5035;
    uint16_t a203 = 0x0120;
    uint16_t a204 = 1;
    uint16_t a205 = 0x0001;
    sdp_data_t *attr200 = sdp_data_alloc(SDP_UINT16, &a200);
    sdp_data_t *attr201 = sdp_data_alloc(SDP_UINT16, &a201);
    sdp_data_t *attr202 = sdp_data_alloc(SDP_UINT16, &a202);
    sdp_data_t *attr203 = sdp_data_alloc(SDP_UINT16, &a203);
    sdp_data_t *attr204 = sdp_data_alloc(SDP_BOOL,   &a204);
    sdp_data_t *attr205 = sdp_data_alloc(SDP_UINT16, &a205);
    sdp_attr_add(record, SDP_ATTR_SPECIFICATION_ID, attr200);
    sdp_attr_add(record, SDP_ATTR_VENDOR_ID, attr201);
    sdp_attr_add(record, SDP_ATTR_PRODUCT_ID, attr202);
    sdp_attr_add(record, SDP_ATTR_VERSION, attr203);
    sdp_attr_add(record, SDP_ATTR_PRIMARY_RECORD, attr204);
    sdp_attr_add(record, SDP_ATTR_VENDOR_ID_SOURCE, attr205);

    sdp_uuid16_create(&profile.uuid, PNP_INFO_PROFILE_ID);
    profile.version = 0x0100;
    pfseq = sdp_list_append(0, &profile);
    sdp_set_profile_descs(record, pfseq);

    int err = 0;
    sdp_session_t *session = 0;

    // connect to the local SDP server, register the service record, and 
    // disconnect
    session = sdp_connect( BDADDR_ANY, BDADDR_LOCAL, SDP_RETRY_IF_BUSY );
    err = sdp_record_register(session, record, 0);

    // cleanup
    sdp_data_free( psm );
    sdp_list_free( l2cap_list, 0 );
    sdp_list_free( sdp_list, 0 );
    sdp_list_free( proto_list, 0);
    sdp_list_free( access_proto_list, 0 );
    sdp_list_free( pfseq, 0 );
    sdp_data_free( attr200 );
    sdp_data_free( attr201 );
    sdp_data_free( attr202 );
    sdp_data_free( attr203 );
    sdp_data_free( attr204 );
    sdp_data_free( attr205 );

    return session;
}

sdp_session_t *register_service2()
{

    uint32_t svc_uuid_int[] = { 0x00000000,0xdefacade,0xafdecade,0xffcacade };
    uint8_t rfcomm_channel = RFCOMM_CHANNEL;
    const char *service_name = "Wireless iAP";
    const char *service_dsc = NULL;
    const char *service_prov = NULL;

    uuid_t root_uuid, l2cap_uuid, rfcomm_uuid, svc_uuid, class_uuid;
    sdp_list_t *l2cap_list = 0, 
               *rfcomm_list = 0,
               *root_list = 0,
               *proto_list = 0, 
               *access_proto_list = 0,
	       *class_list = 0;
    sdp_data_t *channel = 0;

    sdp_record_t *record = sdp_record_alloc();

    /* Force the record to be 0x1000f */
    record->handle = 0x1000f;

    // set the general service ID
    sdp_uuid128_create( &svc_uuid, &svc_uuid_int );
//    sdp_set_service_id( record, svc_uuid );

    // set the Service class ID
    sdp_list_t service_class = {NULL, &svc_uuid};
    sdp_set_service_classes( record, &service_class);

    // make the service record publicly browsable
//    sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
//    root_list = sdp_list_append(0, &root_uuid);
//    sdp_set_browse_groups( record, root_list );

//    sdp_uuid16_create(&class_uuid, PNP_INFO_SVCLASS_ID);
//    class_list = sdp_list_append(0, &class_uuid);
//    sdp_set_service_classes(record, class_list);

    // set l2cap information
    sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
    l2cap_list = sdp_list_append( 0, &l2cap_uuid );
    proto_list = sdp_list_append( 0, l2cap_list );

    // set rfcomm information
    sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
    rfcomm_list = sdp_list_append( 0, &rfcomm_uuid );
    channel = sdp_data_alloc(SDP_UINT8, &rfcomm_channel);
    rfcomm_list = sdp_list_append( rfcomm_list, channel );
    proto_list = sdp_list_append( proto_list, rfcomm_list );

    // attach protocol information to service record
    access_proto_list = sdp_list_append( 0, proto_list );
    sdp_set_access_protos( record, access_proto_list );

    // set the name, provider, and description
    sdp_set_info_attr(record, service_name, service_prov, service_dsc);
    sdp_add_lang_attr(record);
    sdp_set_service_avail(record, 0xff);

    int err = 0;
    sdp_session_t *session = 0;

    // connect to the local SDP server, register the service record, and 
    // disconnect
    session = sdp_connect( BDADDR_ANY, BDADDR_LOCAL, SDP_RETRY_IF_BUSY );
    err = sdp_record_register(session, record, 0);

    // cleanup
    sdp_data_free( channel );
//    sdp_list_free( root_list, 0 );
    sdp_list_free( l2cap_list, 0 );
    sdp_list_free( rfcomm_list, 0 );
    sdp_list_free( proto_list, 0);
    sdp_list_free( access_proto_list, 0 );

    return session;
}

int main(int argc, char **argv)
{
    struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    char buf[1024] = { 0 };
    char cmd[] = { 0x55, 0x4, 0x00, 0x38, 0x00, 0x00, 0xc4};
    int s, client, bytes_read, status;
    socklen_t opt = sizeof(rem_addr);
    sdp_session_t *session1, *session2;

    session1 = register_service1();
    session2 = register_service2();

    // allocate socket
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    // bind socket to port 1 of the first available 
    // local bluetooth adapter
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = (uint8_t) RFCOMM_CHANNEL;
    bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

    // put socket into listening mode
    listen(s, 1);

    // accept one connection
    client = accept(s, (struct sockaddr *)&rem_addr, &opt);

    ba2str( &rem_addr.rc_bdaddr, buf );
    fprintf(stderr, "accepted connection from %s\n", buf);

    action(client, HELI);

    // close connection
    close(client);
    close(s);
    sdp_close(session1);
    sdp_close(session2);
    return 0;
}



