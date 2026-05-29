message(STATUS "Building po files")

find_package(Gettext QUIET)
find_program(XGETTEXT_EXECUTABLE xgettext)
find_program(MSGMERGE_EXECUTABLE msgmerge)
find_program(MSGINIT_EXECUTABLE  msginit)
if (NOT Gettext_FOUND OR NOT XGETTEXT_EXECUTABLE OR NOT MSGMERGE_EXECUTABLE OR NOT MSGINIT_EXECUTABLE)
    message(STATUS "gettext tools not fully available, skipping po build")
    return()
endif()

message(STATUS "gettext found, translation domain: ${PROJECT_NAME}")

#遍历所有需要翻译的文件
file(GLOB_RECURSE _i18n_sources
    RELATIVE "${CMAKE_SOURCE_DIR}"
    CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
    "${CMAKE_SOURCE_DIR}/src/*.ui"
)

#生成POTFILES.in文件
set(_i18n_filelist "${CMAKE_CURRENT_BINARY_DIR}/POTFILES.in")
set(_filelist_contents "")
foreach(_f IN LISTS _i18n_sources)
    set(_filelist_contents "${_filelist_contents}${_f}\n")
endforeach()
file(WRITE "${_i18n_filelist}" "${_filelist_contents}")

#生成.pot模板文件
set(POT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.pot")
add_custom_target(pot-update
    COMMAND ${CMAKE_COMMAND} -E echo "Extracting translatable strings into ${POT_FILE}"
    COMMAND ${XGETTEXT_EXECUTABLE}
        --from-code=UTF-8
        --language=C++
        --add-comments=TRANSLATORS
        --sort-by-file
        --keyword=_
        --keyword=N_
        --package-name=${PROJECT_NAME}
        --package-version=${FREEWB_VERSION}
        --copyright-holder=${PROJECT_NAME}
        --msgid-bugs-address=${PROJECT_NAME}
        --output=${POT_FILE}
        --directory=${CMAKE_SOURCE_DIR}
        --files-from=${_i18n_filelist}
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Updating ${PROJECT_NAME}.pot from source tree"
    VERBATIM
)

#生成po文件
set(FREEWB_LINGUAS zh_CN zh_HK zh_TW)
set(_po_update_commands)
foreach(_lang IN LISTS FREEWB_LINGUAS)
    set(_po "${CMAKE_CURRENT_SOURCE_DIR}/${_lang}.po")
    if (EXISTS "${_po}")
        list(APPEND _po_update_commands
            COMMAND ${CMAKE_COMMAND} -E echo "msgmerge ${_lang}.po"
            COMMAND ${MSGMERGE_EXECUTABLE} --update --backup=none --quiet
                    ${_po} ${POT_FILE}
        )
    else()
        list(APPEND _po_update_commands
            COMMAND ${CMAKE_COMMAND} -E echo "msginit ${_lang}.po"
            COMMAND ${MSGINIT_EXECUTABLE} --input=${POT_FILE} --output-file=${_po}
                    --locale=${_lang} --no-translator
        )
    endif()
endforeach()

add_custom_target(po-update
    ${_po_update_commands}
    DEPENDS pot-update
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Merging ${PROJECT_NAME}.pot into every <lang>.po"
    VERBATIM
)

#编译.po到.mo,并安装到系统locale目录
set(_mo_outputs)
foreach(_lang IN LISTS FREEWB_LINGUAS)
    set(_po_file "${CMAKE_CURRENT_SOURCE_DIR}/${_lang}.po")
    set(_mo "${CMAKE_CURRENT_BINARY_DIR}/${_lang}.mo")

    add_custom_command(
        OUTPUT  "${_mo}"
        COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check --statistics
                --output-file=${_mo} ${_po_file}
        DEPENDS "${_po_file}"
        COMMENT "Compiling ${_lang}.po -> ${_lang}.mo"
        VERBATIM
    )
    list(APPEND _mo_outputs "${_mo}")

    install(FILES "${_mo}"
        DESTINATION "${FREEWB_INSTALL_DATADIR}/locale/${_lang}/LC_MESSAGES"
        RENAME "${PROJECT_NAME}.mo"
    )
endforeach()

if (_mo_outputs)
    add_custom_target(translations ALL DEPENDS ${_mo_outputs})
endif()
