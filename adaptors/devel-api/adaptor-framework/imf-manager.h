#ifndef __DALI_IMF_MANAGER_H__
#define __DALI_IMF_MANAGER_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/dali-signal.h>
#include "input-method-options.h"

namespace Dali
{

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class ImfManager;
}
}

/**
 * @brief The ImfManager class
 *
 * Specifically manages the ecore input method framework which enables the virtual or hardware keyboards.
 */
class DALI_IMPORT_API ImfManager : public BaseHandle
{
public:

  /**
  * @brief The direction of text.
  */
  enum TextDirection
  {
    LeftToRight,
    RightToLeft,
  };

  /**
   * @brief Events that are generated by the IMF.
   */
  enum ImfEvent
  {
    VOID,                ///< No event
    PREEDIT,             ///< Pre-Edit changed
    COMMIT,              ///< Commit recieved
    DELETESURROUNDING,   ///< Event to delete a range of characters from the string
    GETSURROUNDING,      ///< Event to query string and cursor position
    PRIVATECOMMAND       ///< Private command sent from the input panel
  };

  /**
   * @brief Enumeration for state of the input panel.
   */
  enum State
  {
    DEFAULT = 0,   ///< Unknown state
    SHOW,          ///< Input panel is shown
    HIDE,          ///< Input panel is hidden
    WILL_SHOW      ///< Input panel in process of being shown
  };

  /**
   * @brief Enumeration for the type of Keyboard.
   */
  enum KeyboardType
  {
    SOFTWARE_KEYBOARD,  ///< Software keyboard (Virtual keyboard) is default
    HARDWARE_KEYBOARD   ///< Hardware keyboard
  };

  /**
   * @brief This structure is used to pass on data from the IMF regarding predictive text.
   */
  struct ImfEventData
  {
    /**
     * @brief Default Constructor.
     */
    ImfEventData()
    : predictiveString(),
      eventName( VOID ),
      cursorOffset( 0 ),
      numberOfChars ( 0 )
    {
    };

    /**
     * @brief Constructor
     *
     * @param[in] aEventName The name of the event from the IMF.
     * @param[in] aPredictiveString The pre-edit or commit string.
     * @param[in] aCursorOffset Start position from the current cursor position to start deleting characters.
     * @param[in] aNumberOfChars The number of characters to delete from the cursorOffset.
     */
    ImfEventData( ImfEvent aEventName, const std::string& aPredictiveString, int aCursorOffset, int aNumberOfChars )
    : predictiveString( aPredictiveString ),
      eventName( aEventName ),
      cursorOffset( aCursorOffset ),
      numberOfChars( aNumberOfChars )
    {
    }

    // Data
    std::string predictiveString; ///< The pre-edit or commit string.
    ImfEvent eventName;           ///< The name of the event from the IMF.
    int cursorOffset;             ///< Start position from the current cursor position to start deleting characters.
    int numberOfChars;            ///< number of characters to delete from the cursorOffset.
  };

  /**
   * @brief Data required by IMF from the callback
   */
  struct ImfCallbackData
  {
    /**
     * @brief Constructor
     */
    ImfCallbackData()
    : currentText(),
      cursorPosition( 0 ),
      update( false ),
      preeditResetRequired( false )
    {
    }

    /**
     * @brief Constructor
     * @param[in] aUpdate True if cursor position needs to be updated
     * @param[in] aCursorPosition new position of cursor
     * @param[in] aCurrentText current text string
     * @param[in] aPreeditResetRequired flag if preedit reset is required.
     */
    ImfCallbackData( bool aUpdate, int aCursorPosition, const std::string& aCurrentText, bool aPreeditResetRequired )
    : currentText( aCurrentText ),
      cursorPosition( aCursorPosition ),
      update( aUpdate ),
      preeditResetRequired( aPreeditResetRequired )
    {
    }

    std::string currentText;      ///< current text string
    int cursorPosition;           ///< new position of cursor
    bool update               :1; ///< if cursor position needs to be updated
    bool preeditResetRequired :1; ///< flag if preedit reset is required.
  };

  typedef Signal< void (ImfManager&) > ImfManagerSignalType; ///< Keyboard actived signal
  typedef Signal< ImfCallbackData ( ImfManager&, const ImfEventData& ) > ImfEventSignalType; ///< keyboard events
  typedef Signal< void () > VoidSignalType;
  typedef Signal< void (bool) > StatusSignalType;
  typedef Signal< void (KeyboardType) > KeyboardTypeSignalType; ///< keyboard type

public:

  /**
   * @brief Retrieve a handle to the instance of ImfManager.
   * @return A handle to the ImfManager.
   */
  static ImfManager Get();

  /**
   * @brief Activate the IMF.
   *
   * It means that the text editing is started at somewhere.
   * If the H/W keyboard isn't connected then it will show the virtual keyboard.
   */
  void Activate();

  /**
   * @brief Deactivate the IMF.
   *
   * It means that the text editing is finished at somewhere.
   */
  void Deactivate();

  /**
   * @brief Get the restoration status, which controls if the keyboard is restored after the focus lost then regained.
   *
   * If true then keyboard will be restored (activated) after focus is regained.
   * @return restoration status.
   */
  bool RestoreAfterFocusLost() const;

  /**
   * @brief Set status whether the IMF has to restore the keyboard after losing focus.
   *
   * @param[in] toggle True means that keyboard should be restored after focus lost and regained.
   */
  void SetRestoreAfterFocusLost( bool toggle );

  /**
   * @brief Send message reset the pred-edit state / imf module.
   *
   * Used to interupt pre-edit state maybe due to a touch input.
   */
  void Reset();

  /**
   * @brief Notifies IMF context that the cursor position has changed, required for features like auto-capitalisation.
   */
  void NotifyCursorPosition();

  /**
   * @brief Sets cursor position stored in VirtualKeyboard, this is required by the IMF context.
   *
   * @param[in] cursorPosition position of cursor
   */
  void SetCursorPosition( unsigned int cursorPosition );

  /**
   * @brief Gets cursor position stored in VirtualKeyboard, this is required by the IMF context.
   *
   * @return current position of cursor
   */
  unsigned int GetCursorPosition() const;

  /**
   * @brief Method to store the string required by the IMF, this is used to provide predictive word suggestions.
   *
   * @param[in] text The text string surrounding the current cursor point.
   */
  void SetSurroundingText( const std::string& text );

  /**
   * @brief Gets current text string set within the IMF manager, this is used to offer predictive suggestions.
   *
   * @return current position of cursor
   */
  const std::string& GetSurroundingText() const;

  /**
 * @brief Notifies IMF context that text input is set to multi line or not
 *
 * @param[in] multiLine True if multiline text input is used
 */
  void NotifyTextInputMultiLine( bool multiLine );

  /**
   * @brief Returns text direction of the keyboard's current input language.
   * @return The direction of the text.
   */
  TextDirection GetTextDirection();

  /**
   * @brief Provides size and position of keyboard.
   *
   * Position is relative to whether keyboard is visible or not.
   * If keyboard is not visible then position will be off the screen.
   * If keyboard is not being shown when this method is called the keyboard is partially setup (IMFContext) to get
   * the values then taken down.  So ideally GetInputMethodArea() should be called after Show().
   * @return rect which is keyboard panel x, y, width, height
   */
  Dali::Rect<int> GetInputMethodArea();

  /**
   * @brief Set one or more of the Input Method options
   * @param[in] options The options to be applied
   */
  void ApplyOptions( const InputMethodOptions& options );

  /**
   * @brief Sets up the input-panel specific data.
   * @param[in] data The specific data to be set to the input panel
   */
  void SetInputPanelData( const std::string& data );

  /**
   * @brief Gets the specific data of the current active input panel.
   *
   * Input Panel Data is not always the data which is set by SetInputPanelData().
   * Data can be changed internally in the input panel.
   * It is just used to get a specific data from the input panel to an application.
   * @param[in] data The specific data to be got from the input panel
   */
  void GetInputPanelData( std::string& data );

  /**
   * @brief Gets the state of the current active input panel.
   * @return The state of the input panel.
   */
  State GetInputPanelState();

  /**
   * @brief Sets the return key on the input panel to be visible or invisible.
   *
   * The default is true.
   * @param[in] visible True if the return key is visible(enabled), false otherwise.
   */
  void SetReturnKeyState( bool visible );

  /**
   * @brief Enable to show the input panel automatically when focused.
   * @param[in] enabled If true, the input panel will be shown when focused
   */
  void AutoEnableInputPanel( bool enabled );

  /**
   * @brief Shows the input panel.
   */
  void ShowInputPanel();

  /**
   * @brief Hides the input panel.
   */
  void HideInputPanel();

  /**
   * @brief Gets the keyboard type.
   *
   * The default keyboard type is SOFTWARE_KEYBOARD.
   * @return The keyboard type
   */
  KeyboardType GetKeyboardType();

  /**
   * @brief Gets the current language locale of the input panel.
   *
   * ex) en_US, en_GB, en_PH, fr_FR, ...
   * @return The current language locale of the input panel
   */
  std::string GetInputPanelLocale();

public:

  // Signals

  /**
   * @brief This is emitted when the virtual keyboard is connected to or the hardware keyboard is activated.
   *
   * @return The IMF Activated signal.
   */
  ImfManagerSignalType& ActivatedSignal();

  /**
   * @brief This is emitted when the IMF manager receives an event from the IMF.
   *
   * @return The Event signal containing the event data.
   */
  ImfEventSignalType& EventReceivedSignal();

  /**
   * @brief Connect to this signal to be notified when the virtual keyboard is shown or hidden.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(bool keyboardShown);
   * @endcode
   * If the parameter keyboardShown is true, then the keyboard has just shown, if it is false, then it
   * has just been hidden.
   * @return The signal to connect to.
   */
  StatusSignalType& StatusChangedSignal();

  /**
   * @brief Connect to this signal to be notified when the virtual keyboard is resized.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName();
   * @endcode
   * User can get changed size by using GetInputMethodArea() in the callback
   * @return The signal to connect to.
   */
  VoidSignalType& ResizedSignal();

  /**
   * @brief Connect to this signal to be notified when the virtual keyboard's language is changed.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName();
   * @endcode
   * User can get the text direction of the language by calling GetTextDirection() in the callback.
   * @return The signal to connect to.
   */
  VoidSignalType& LanguageChangedSignal();

  /**
   * @brief Connect to this signal to be notified when the keyboard type is changed.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName( KeyboardType keyboard );
   * @endcode
   *
   * @return The signal to connect to.
   */
  KeyboardTypeSignalType& KeyboardTypeChangedSignal();

  // Construction & Destruction

  /**
   * @brief Constructor.
   */
  ImfManager();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~ImfManager();

  /**
   * @brief This constructor is used by ImfManager::Get().
   *
   * @param[in] imfManager A pointer to the imf Manager.
   */
  explicit DALI_INTERNAL ImfManager( Internal::Adaptor::ImfManager* imfManager );
};

} // namespace Dali

#endif // __DALI_IMF_MANAGER_H__
