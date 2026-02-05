Feature: Brake light

    Scenario: Brake light toggling (Off)
        Given on setup

        When I enable schedule 'NormalSchedule' on 'LineMaster'
        And I set master signal 'LightSynchronization.LightMode' to 'Off' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '0' on 'LineMaster'
        And I wait '2' seconds

        And I set master signal 'SpeedStatus.BrakeState' to 'Braking' on 'LineMaster'
        And I wait '500' milliseconds

        #Then the brightness of 'BrakeLight' should be '100' on 'LineMaster'

    Scenario: Brake light toggling (Standard)
        Given on setup

        When I enable schedule 'NormalSchedule' on 'LineMaster'
        And I set master signal 'LightSynchronization.LightMode' to 'Standard' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '10' on 'LineMaster'
        And I wait '2' seconds

        And I set master signal 'SpeedStatus.BrakeState' to 'Braking' on 'LineMaster'
        And I wait '500' milliseconds

        #Then the brightness of 'BrakeLight' should be '100' on 'LineMaster'

    Scenario: Brake light toggling (Adaptive)
        Given on setup

        When I enable schedule 'NormalSchedule' on 'LineMaster'
        And I set master signal 'LightSynchronization.LightMode' to 'Adaptive' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '30' on 'LineMaster'
        And I wait '2' seconds

        And I set master signal 'SpeedStatus.BrakeState' to 'Braking' on 'LineMaster'
        And I wait '500' milliseconds

        #Then the brightness of 'BrakeLight' should be '100' on 'LineMaster'

    Scenario: Brake light toggling (Emergency)
        Given on setup

        When I enable schedule 'NormalSchedule' on 'LineMaster'
        And I set master signal 'LightSynchronization.LightMode' to 'Emergency' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '0' on 'LineMaster'
        And I wait '2' seconds

        And I set master signal 'SpeedStatus.BrakeState' to 'Braking' on 'LineMaster'
        And I wait '500' milliseconds

        #Then the brightness of 'BrakeLight' should be '0' on 'LineMaster'

    Scenario: Brake light toggling (Safety)
        Given on setup

        # TODO: enable speed status
        And I wait '5' seconds

        And I set master signal 'SpeedStatus.BrakeState' to 'Braking' on 'LineMaster'
        And I wait '500' milliseconds

        #Then the brightness of 'BrakeLight' should be '0' on 'LineMaster'

    Scenario: Brake light disabled
        Given on setup

        When I enable schedule 'NormalSchedule' on 'LineMaster'
        And I set master signal 'LightSynchronization.LightMode' to 'Emergency' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '0' on 'LineMaster'
        And I set master signal 'RearLightSetting.BrakeLightMode' to 'Off' on 'LineMaster'
        And I wait '2' seconds

        And I set master signal 'SpeedStatus.BrakeState' to 'Braking' on 'LineMaster'
        And I wait '500' milliseconds

        #Then the brightness of 'BrakeLight' should be '0' on 'LineMaster'

    Scenario: Brake signal invalid

    Scenario: Brake signal timeout
