/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NoBlockingSocketException.hpp                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akhouya <akhouya@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/19 14:16:07 by akhouya           #+#    #+#             */
/*   Updated: 2023/05/19 14:16:49 by akhouya          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <exception>
#include <string>
class SetNonBlockingException : public std::exception {
public:
    SetNonBlockingException(const std::string& message) : errorMessage(message) {}

    const char* what() const noexcept override {
        return errorMessage.c_str();
    }

private:
    std::string errorMessage;
};

class ConnectionFailedException : public std::exception {
public:
    ConnectionFailedException(const std::string& message) : errorMessage(message) {}

    const char* what() const noexcept override {
        return errorMessage.c_str();
    }

private:
    std::string errorMessage;
};