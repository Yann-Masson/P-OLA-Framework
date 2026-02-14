/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** UserPreferenceService
*/

#pragma once
#include "AInputService.hpp"

struct UserPreferenceData {
    double preferredTemperature;
    bool isHome;
};

class UserPreferenceService: public AInputService<UserPreferenceData> {
    public:
        using AInputService<UserPreferenceData>::AInputService;
        UserPreferenceData getInput() override;

    protected:
    private:
};
