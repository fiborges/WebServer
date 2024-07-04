/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CR.hpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 14:07:37 by brolivei          #+#    #+#             */
/*   Updated: 2024/07/04 15:56:37 by brolivei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CR_HPP
#define CR_HPP

#include <iostream>

class	CR
{
	private:
		
	public:
		CR();
		~CR();

		bool	ItIsChunked(std::string& Request);
};

#endif
