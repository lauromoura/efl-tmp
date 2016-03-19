#include "elm_entry.eo.legacy.h"

/**
 * This adds an entry to @p parent object.
 *
 * By default, entries are:
 * @li not scrolled
 * @li multi-line
 * @li word wrapped
 * @li autosave is enabled
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Elm_Entry
 */
EAPI Evas_Object       *elm_entry_add(Evas_Object *parent);

/**
 * This sets the text displayed within the entry to @p entry.
 *
 * @param obj The entry object
 * @param entry The text to be displayed
 *
 * @note Using this function bypasses text filters
 *
 * @ingroup Elm_Entry
 */
EAPI void               elm_entry_entry_set(Evas_Object *obj, const char *entry);

/**
 * This returns the text currently shown in object @p entry.
 * See also elm_entry_entry_set().
 *
 * @param obj The entry object
 * @return The currently displayed text or NULL on failure
 *
 * @ingroup Elm_Entry
 */
EAPI const char        *elm_entry_entry_get(const Evas_Object *obj);

/**
 *
 * This sets the file (and implicitly loads it) for the text to display and
 * then edit. All changes are written back to the file after a short delay if
 * the entry object is set to autosave (which is the default).
 *
 * If the entry had any other file set previously, any changes made to it
 * will be saved if the autosave feature is enabled, otherwise, the file
 * will be silently discarded and any non-saved changes will be lost.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * @ingroup Elm_Entry
 *
 * @param[in] file The path to the file to load and save
 * @param[in] format The file format
 */
EAPI Eina_Bool          elm_entry_file_set(Evas_Object *obj, const char *file, Elm_Text_Format format);

/**
 *
 * Get the file being edited by the entry.
 *
 * This function can be used to retrieve any file set on the entry for
 * edition, along with the format used to load and save it.
 *
 * @ingroup Elm_Entry
 *
 * @param[out] file The path to the file to load and save
 * @param[out] format The file format
 */
EAPI void               elm_entry_file_get(const Evas_Object *obj, const char **file, Elm_Text_Format *format);

/**
 * @brief Sets if the entry is to be editable or not.
 *
 * By default, entries are editable and when focused, any text input by the
 * user will be inserted at the current cursor position. But calling this
 * function with @c editable as @c false will prevent the user from inputting
 * text into the entry.
 *
 * The only way to change the text of a non-editable entry is to use @ref
 * elm_object_text_set, @ref elm_entry_entry_insert and other related
 * functions.
 *
 * @param[in] editable If @c true, user input will be inserted in the entry, if
 * not, the entry is read-only and no user input is allowed.
 *
 * @ingroup Elm_Entry
 */
EAPI void elm_entry_editable_set(Evas_Object *obj, Eina_Bool editable);

/**
 * @brief Get whether the entry is editable or not.
 *
 * @return If @c true, user input will be inserted in the entry, if not, the
 * entry is read-only and no user input is allowed.
 *
 * @ingroup Elm_Entry
 */
EAPI Eina_Bool elm_entry_editable_get(const Evas_Object *obj);
