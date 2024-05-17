/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LOG_CLASS.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 11:30:14 by brolivei          #+#    #+#             */
/*   Updated: 2024/05/17 11:40:34 by brolivei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LOG_CLASS.hpp"

LOG_CLASS::LOG_CLASS()
{}

LOG_CLASS::~LOG_CLASS()
{}

void	LOG_CLASS::CreateLog(const char* fileName, std::string& Content)
{
	std::ofstream	out(fileName, std::ios::binary | std::ios::app);

	if (!out.is_open())
		return ;

	out.write(Content.data(), Content.size());

	out.close();
}
