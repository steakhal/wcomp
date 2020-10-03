flex_target(flex_scanner tokens.l "${GENERATED_SRC}/tokens.cpp")
bison_target(grammer grammer.yy "${GENERATED_SRC}/grammer.cpp")


add_executable(wcomp2 parser-main.cpp)
target_link_libraries(wcomp2 PRIVATE CONAN_PKG::bison)


add_executable(wcomp "${GENERATED_SRC}/tokens.cpp")
target_link_libraries(wcomp PRIVATE CONAN_PKG::flex)
