idf_component_register(
    SRCS "main.cc" "functionblocks.cc" "plcmanager.cc"
    PRIV_INCLUDE_DIRS "c:/opt/flatbuffers/include"
    EMBED_FILES "../files2embed/defaultfunctionblockdiagram.data")