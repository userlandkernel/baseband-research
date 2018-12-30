<interface name="audio">
    <method rtype="int" name="get_info">
        <arg atype="struct audio_info *" name="info" />
    </method>

   <method rtype="int" name="mixer_get_channels"/>

   <method rtype="int" name="mixer_probe_channel">
    <arg atype="uint32_t" name="chan" />
    <arg atype="uint32_t*" name="min" />
    <arg atype="uint32_t*" name="max" />
    <arg atype="uint32_t*" name="ctrl" />
   </method>

   <method rtype="int" name="mixer_control_write">
    <arg atype="uint8_t" name="chan" />
    <arg atype="uint32_t" name="control" />
   </method>

   <method rtype="int" name="mixer_control_read">
    <arg atype="uint8_t" name="chan" />
    <arg atype="uint32_t*" name="control" />
   </method>

   <method rtype="int" name="power_on">
   </method>

   <method rtype="int" name="get_config">
     <arg atype="struct audio_buf_conf*" name="conf" />
   </method>

   <method rtype="int" name="open_stream">
     <arg atype="uint8_t" name="stream" />
     <arg atype="struct audio_buf_conf*" name="conf" />
   </method>

   <method rtype="int" name="close_stream">
     <arg atype="uint8_t" name="stream" />
   </method>

   <method rtype="int" name="config_stream">
     <arg atype="uint8_t" name="stream" />
     <arg atype="struct audio_stream_conf*" name="conf" />
   </method>

   <method rtype="int" name="control_stream">
     <arg atype="uint8_t" name="stream" />
     <arg atype="uintptr_t" name="cmd" />
   </method>

   <method rtype="uintptr_t" name="get_stream_pos">
     <arg atype="uint8_t" name="stream" />
   </method>
</interface>
