from pytest_bdd import scenarios, given, when, then

scenarios('features/state/test_SystemStates.feature')
scenarios('features/state/test_OperationStatus.feature')
scenarios('features/state/test_SleepWakeup.feature')

scenarios('features/lights/test_BrightnessControl.feature')
scenarios('features/lights/test_BrightnessSweep.feature')
scenarios('features/lights/test_Strobe.feature')

scenarios('features/diagnostics/test_SwNumber.feature')
