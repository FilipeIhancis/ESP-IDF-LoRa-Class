set(component_srcs "lora.cpp")

idf_component_register(SRCS "${component_srcs}"
                       PRIV_REQUIRES driver
                       INCLUDE_DIRS "include")
