Feature: Brightness Control
    # Test 9: Emergency, Target=0
    # Test 10: Emergency, Target=100

    Scenario: Off, Target=0
        Given on setup
        #And default config

        When I enable schedule 'LightSchedule' on 'LineMaster'
        And I wait '500' milliseconds
        And I set master signal 'LightSynchronization.LightMode' to 'Off' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '0' on 'LineMaster'
        And I wait '5' seconds

        # TODO: somehow check output brightness
        #Then the brightness should be '0'

    Scenario: Off, Target=100
        Given on setup
        #And default config

        When I enable schedule 'LightSchedule' on 'LineMaster'
        And I wait '500' milliseconds
        And I set master signal 'LightSynchronization.LightMode' to 'Off' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '100' on 'LineMaster'
        And I wait '5' seconds

        # TODO: somehow check output brightness
        #Then the brightness should be '0'

    Scenario: Adaptive, Target=0
        Given on setup
        #And default config

        When I enable schedule 'LightSchedule' on 'LineMaster'
        And I wait '500' milliseconds
        And I set master signal 'LightSynchronization.LightMode' to 'Adaptive' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '0' on 'LineMaster'
        And I wait '5' seconds

        # TODO: somehow check output brightness
        #Then the brightness should be '0'

    Scenario: Adaptive, Target=50
        Given on setup
        #And default config

        When I enable schedule 'LightSchedule' on 'LineMaster'
        And I wait '500' milliseconds
        And I set master signal 'LightSynchronization.LightMode' to 'Adaptive' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '50' on 'LineMaster'
        And I wait '5' seconds

        # TODO: somehow check output brightness
        #Then the brightness should be around '50'

    Scenario: Adaptive, Target=100
        Given on setup
        #And default config

        When I enable schedule 'LightSchedule' on 'LineMaster'
        And I wait '500' milliseconds
        And I set master signal 'LightSynchronization.LightMode' to 'Adaptive' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '100' on 'LineMaster'
        And I wait '5' seconds

        # TODO: somehow check output brightness
        #Then the brightness should be around '100'

    Scenario: Standard, Target=0
        Given on setup
        #And default config

        When I enable schedule 'LightSchedule' on 'LineMaster'
        And I wait '500' milliseconds
        And I set master signal 'LightSynchronization.LightMode' to 'Standard' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '0' on 'LineMaster'
        And I wait '5' seconds

        # TODO: somehow check output brightness
        #Then the brightness should be '20'

    Scenario: Standard, Target=50
        Given on setup
        #And default config

        When I enable schedule 'FrontLightSchedule' on 'LineMaster'
        And I wait '500' milliseconds
        And I set master signal 'LightSynchronization.LightMode' to 'Standard' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '50' on 'LineMaster'
        And I wait '5' seconds

        # TODO: somehow check output brightness
        #Then the brightness should be around '50'

    Scenario: Standard, Target=100
        Given on setup

        When I enable schedule 'LightSchedule' on 'LineMaster'
        And I wait '500' milliseconds
        And I set master signal 'LightSynchronization.LightMode' to 'Standard' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '100' on 'LineMaster'
        And I wait '5' seconds

        # TODO: somehow check output brightness
        #Then the brightness should be around '100'

    Scenario: Emergency, Target=0
        Given on setup

        When I enable schedule 'LightSchedule' on 'LineMaster'
        And I wait '500' milliseconds
        And I set master signal 'LightSynchronization.LightMode' to 'Emergency' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '0' on 'LineMaster'
        And I wait '5' seconds

        # TODO: somehow check output brightness
        #Then the brightness should be around '30'

    Scenario: Emergency, Target=100
        Given on setup

        When I enable schedule 'LightSchedule' on 'LineMaster'
        And I wait '500' milliseconds
        And I set master signal 'LightSynchronization.LightMode' to 'Emergency' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '100' on 'LineMaster'
        And I wait '5' seconds

        # TODO: somehow check output brightness
        #Then the brightness should be around '30'
