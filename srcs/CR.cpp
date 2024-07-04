/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CR.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 14:51:17 by brolivei          #+#    #+#             */
/*   Updated: 2024/07/04 15:54:49 by brolivei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CR.hpp"

CR::CR() {}

CR::~CR() {}

bool	CR::ItIsChunked(std::string& Request)
{
	std::string	ToFind = "Transfer-Encoding: chunked";

	if (Request.find(ToFind))
		return (true);
	else
		return (false);
}
