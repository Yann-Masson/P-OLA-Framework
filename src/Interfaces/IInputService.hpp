/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** IInputService
*/

#pragma once

template<typename T>
class IInputService {
    public:
        virtual ~IInputService() = default;
        virtual T getInput() = 0;
};
