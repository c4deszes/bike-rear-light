Feature: Brightness sweep

    Scenario: Adaptive sweep
        Given on setup

        When I enable schedule 'LightSchedule' on 'LineMaster'
        And I set master signal 'LightSynchronization.LightMode' to 'Adaptive' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '0' on 'LineMaster'
        And I wait '2' seconds

        And I sweep the brightness from '0' to '100' with '1' increments and '200ms' delay

        #Then the operation status of 'FrontLight' should be 'Ok' on 'LineMaster'

    Scenario: Standard sweep
        Given on setup

        When I enable schedule 'LightSchedule' on 'LineMaster'
        And I set master signal 'LightSynchronization.LightMode' to 'Standard' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '0' on 'LineMaster'
        And I wait '2' seconds

        And I sweep the brightness from '0' to '100' with '1' increments and '200ms' delay

        #Then the operation status of 'FrontLight' should be 'Ok' on 'LineMaster'
