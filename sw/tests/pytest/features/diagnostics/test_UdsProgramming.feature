Feature: Uds Programming

    Scenario: Set standard level
        Given on setup

        # Change brightness to 300
        When I wait '5' seconds
        When I write property 'Brightness_LevelSafety' of 'RearLight' to '500' on 'UDS'
        When I wait '5' seconds
        When I send shutdown request on 'LineMaster'
        When I wait '5' seconds
        When I send wakeup request on 'LineMaster'
        When I wait '5' seconds

        # Brightness should be 300

        # When I send wakeup request on 'LineMaster'
        # And I wait '1' seconds
        # And I write property 'Brightness_LevelSafety' of 'RearLight' to '100' on 'UDS'
        # And I wait '5' seconds

        # When I send shutdown request on 'LineMaster'
        # And I wait '5' seconds
        # And I send wakeup request on 'LineMaster'
